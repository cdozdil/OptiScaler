#ifndef IMTERM_TERMINAL_HPP
#define IMTERM_TERMINAL_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,         ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  is furnished to do so, subject to the following conditions:                                                                        ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <array>
#include <imgui.h>

#include "utils.hpp"
#include "misc.hpp"

#ifdef IMTERM_USE_FMT
#include "fmt/format.h"
#endif

namespace ImTerm {

	// checking that you can use a given class as a TerminalHelper
	// giving human-friendlier error messages than just letting the compiler explode
	namespace details {
		template<typename TerminalHelper, typename CommandTypeCref>
		struct assert_wellformed {

			template<typename T>
			using find_commands_by_prefix_method_v1 =
			decltype(std::declval<T&>().find_commands_by_prefix(std::declval<std::string_view>()));

			template<typename T>
			using find_commands_by_prefix_method_v2 =
			decltype(std::declval<T&>().find_commands_by_prefix(std::declval<const char *>(),
			                                                    std::declval<const char *>()));

			template<typename T>
			using list_commands_method = decltype(std::declval<T&>().list_commands());

			template<typename T>
			using format_method = decltype(std::declval<T&>().format(std::declval<std::string>(), std::declval<message::type>()));

			static_assert(
					misc::is_detected_with_return_type_v<find_commands_by_prefix_method_v1, std::vector<CommandTypeCref>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::vector<command_type_cref> find_command_by_prefix(std::string_view)'. "
					"See term::terminal_helper_example for reference");
			static_assert(
					misc::is_detected_with_return_type_v<find_commands_by_prefix_method_v2, std::vector<CommandTypeCref>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::vector<command_type_cref> find_command_by_prefix(const char*, const char*)'. "
					"See term::terminal_helper_example for reference");
			static_assert(
					misc::is_detected_with_return_type_v<list_commands_method, std::vector<CommandTypeCref>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::vector<command_type_cref> list_commands()'. "
					"See term::terminal_helper_example for reference");
			static_assert(
					misc::is_detected_with_return_type_v<format_method, std::optional<message>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::optional<term::message> format(std::string, term::message::type)'. "
					"See term::terminal_helper_example for reference");
		};
	}

	template<typename TerminalHelper>
	class terminal {
		using buffer_type = std::array<char, 1024>;
		using small_buffer_type = std::array<char, 128>;
	public:
		using value_type = misc::non_void_t<typename TerminalHelper::value_type>;
		using command_type = command_t<terminal<TerminalHelper>>;
		using command_type_cref = std::reference_wrapper<const command_type>;
		using argument_type = argument_t<terminal>;

		using terminal_helper_is_valid = details::assert_wellformed<TerminalHelper, command_type_cref>;

		inline static const std::vector<config_panels> DEFAULT_ORDER = {config_panels::clearbutton,
				  config_panels::autoscroll, config_panels::autowrap, config_panels::long_filter, config_panels::loglevel};

		// You shall call this constructor you used a non void value_type
		template <typename T = value_type, typename = std::enable_if_t<!std::is_same_v<T, misc::details::structured_void>>>
		explicit terminal(value_type& arg_value, const char * window_name_ = "terminal", int base_width_ = 900,
		                  int base_height_ = 200, std::shared_ptr<TerminalHelper> th = std::make_shared<TerminalHelper>())
				: terminal(arg_value, window_name_, base_width_, base_height_, std::move(th), terminal_helper_is_valid{}) {}


		// You shall call this constructor you used a void value_type
		template <typename T = value_type, typename = std::enable_if_t<std::is_same_v<T, misc::details::structured_void>>>
		explicit terminal(const char * window_name_ = "terminal", int base_width_ = 900,
		                  int base_height_ = 200, std::shared_ptr<TerminalHelper> th = std::make_shared<TerminalHelper>())
		                  : terminal(misc::details::no_value, window_name_, base_width_, base_height_, std::move(th), terminal_helper_is_valid{}) {}

		// Returns the underlying terminal helper
		std::shared_ptr<TerminalHelper> get_terminal_helper() {
			return m_t_helper;
		}

		// shows the terminal. Call at each frame (in a more ImGui style, this would be something like ImGui::terminal(....);
		// returns true if the terminal thinks it should still be displayed next frame, false if it thinks it should be hidden
		// return value is true except if a command required a close, or if the "escape" key was pressed.
		bool show(const std::vector<config_panels>& panels_order = DEFAULT_ORDER) noexcept;

		// returns the command line history
		const std::vector<std::string>& get_history() const noexcept {
			return m_command_history;
		}

		// if invoked, the next call to "show" will return false
		void set_should_close() noexcept {
			m_close_request = true;
		}

		// clears all theme's optionals
		void reset_colors() noexcept;

		// returns current theme
		struct theme& theme() {
			return m_colors;
		}

		// set whether the autocompletion OSD should be above/under the text input, or if it should be disabled
		void set_autocomplete_pos(position p) {
			m_autocomplete_pos = p;
		}

		// returns current autocompletion position
		position get_autocomplete_pos() const {
			return m_autocomplete_pos;
		}

#ifdef IMTERM_USE_FMT
		// logs a colorless text to the message panel
		// added as terminal message with info severity
		template <typename... Args>
		void add_formatted(const char* fmt, Args&&... args) {
			add_text(fmt::format(fmt, std::forward<Args>(args)...));
		}

		// logs a colorless text to the message panel
		// added as terminal message with warn severity
		template <typename... Args>
		void add_formatted_err(const char* fmt, Args&&... args) {
			add_text_err(fmt::format(fmt, std::forward<Args>(args)...));
		}
#endif

		// logs a text to the message panel
		// added as terminal message with info severity
		void add_text(std::string str, unsigned int color_beg, unsigned int color_end);

		// logs a text to the message panel, color spans from color_beg to the end of the message
		// added as terminal message with info severity
		void add_text(std::string str, unsigned int color_beg) {
			add_text(str, color_beg, static_cast<unsigned>(str.size()));
		}

		// logs a colorless text to the message panel
		// added as a terminal message with info severity,
		void add_text(std::string str) {
			add_text(str, 0, 0);
		}

		// logs a text to the message panel
		// added as terminal message with warn severity
		void add_text_err(std::string str, unsigned int color_beg, unsigned int color_end);

		// logs a text to the message panel, color spans from color_beg to the end of the message
		// added as terminal message with warn severity
		void add_text_err(std::string str, unsigned int color_beg) {
			add_text_err(str, color_beg, static_cast<unsigned>(str.size()));
		}

		// logs a colorless text to the message panel
		// added as terminal message with warn severity
		void add_text_err(std::string str) {
			add_text_err(str, 0, 0);
		}

		// logs a message to the message panel
		void add_message(const message& msg) {
			add_message(message{msg});
		}
		void add_message(message&& msg);

		// clears the message panel
		void clear();

		message::severity::severity_t log_level() noexcept {
			return static_cast<message::severity::severity_t>(m_level + static_cast<int>(m_lowest_log_level_val));
		}

		void log_level(message::severity::severity_t new_level) noexcept {
			if (m_lowest_log_level_val > new_level) {
				set_min_log_level(new_level);
			}
			m_level = new_level - m_lowest_log_level_val;
		}

		// returns the text used to label the button that clears the message panel
		// set it to an empty optional if you don't want the button to be displayed
		std::optional<std::string>& clear_text() noexcept {
			return m_clear_text;
		}

		// returns the text used to label the checkbox enabling or disabling message panel auto scrolling
		// set it to an empty optional if you don't want the checkbox to be displayed
		std::optional<std::string>& autoscroll_text() noexcept {
			return m_autoscroll_text;
		}

		// returns the text used to label the checkbox enabling or disabling message panel text auto wrap
		// set it to an empty optional if you don't want the checkbox to be displayed
		std::optional<std::string>& autowrap_text() noexcept {
			return m_autowrap_text;
		}

		// returns the text used to label the drop down list used to select the minimum severity to be displayed
		// set it to an empty optional if you don't want the drop down list to be displayed
		std::optional<std::string>& log_level_text() noexcept {
			return m_log_level_text;
		}

		// returns the text used to label the text input used to filter out logs
		// set it to an empty optional if you don't want the filter to be displayed
		std::optional<std::string>& filter_hint() noexcept {
			return m_filter_hint;
		}

		// allows you to set the text in the log_level drop down list
		// the std::string_view/s are copied, so you don't need to manage their life-time
		// set log_level_text() to an empty optional if you want to disable the drop down list
		void set_level_list_text(std::string_view trace_str, std::string_view debug_str, std::string_view info_str,
					std::string_view warn_str, std::string_view err_str, std::string_view critical_str, std::string_view none_str);

		// sets the maximum verbosity a user can set in the terminal with the log_level drop down list
		// for instance, if you pass 'info', the user will be able to select 'info','warning','error', 'critical', and 'none',
		// but will never be able to view 'trace' and 'debug' messages
		void set_min_log_level(message::severity::severity_t level);

		// Adds custom flags to the terminal window
		void set_flags(ImGuiWindowFlags flags) noexcept {
			m_flags = flags;
		}

		// Sets the maximum number of saved messages
		void set_max_log_len(std::vector<message>::size_type max_size);

		// Sets the size of the terminal
		void set_size(unsigned int x, unsigned int y) noexcept {
			set_width(x);
			set_height(y);
		}

		void set_size(ImVec2 sz) noexcept {
			assert(sz.x >= 0);
			assert(sz.y >= 0);
			set_size(static_cast<unsigned>(sz.x), static_cast<unsigned>(sz.y));
		}

		void set_width(unsigned int x) noexcept {
			m_base_width = x;
			m_update_width = true;
		}

		void set_height(unsigned int y) noexcept {
			m_base_height = y;
			m_update_height = true;
		}

		// Get last window size
		ImVec2 get_size() const noexcept {
			return m_current_size;
		}

		// Allows / disallow resize on a given axis
		void allow_x_resize() noexcept {
			m_allow_x_resize = true;
		}

		void allow_y_resize() noexcept {
			m_allow_y_resize = true;
		}
		void disallow_x_resize() noexcept {
			m_allow_x_resize = false;
		}

		void disallow_y_resize() noexcept {
			m_allow_y_resize = false;
		}
		void set_x_resize_allowance(bool allowed) noexcept {
			m_allow_x_resize = allowed;
		}

		void set_y_resize_allowance(bool allowed) noexcept {
			m_allow_y_resize = allowed;
		}

	    // executes a statement, simulating user input
	    // returns false if given string is too long to be interpreted
	    // if true is returned, any text inputed by the user is overridden
	    bool execute(std::string_view str) noexcept {
		    if (str.size() <= m_command_buffer.size()) {
			    std::copy(str.begin(), str.end(), m_command_buffer.begin());
                m_buffer_usage = str.size();
                call_command();
			    m_buffer_usage = 0;
			    m_command_buffer[0] = '\0';
			    return true;
            }
		    return false;
	    }

	private:
		explicit terminal(value_type& arg_value, const char * window_name_, int base_width_, int base_height_, std::shared_ptr<TerminalHelper> th, terminal_helper_is_valid&&);

		void try_log(std::string_view str, message::type type);

		void compute_text_size() noexcept;

		void display_settings_bar(const std::vector<config_panels>& panels_order) noexcept;

		void display_messages() noexcept;

		void display_command_line() noexcept;

		// displaying command_line itself
		void show_input_text() noexcept;

		void handle_unfocus() noexcept;

		void show_autocomplete() noexcept;

		void call_command() noexcept;

		void push_message(message&&);

		std::optional<std::string> resolve_history_reference(std::string_view str, bool& modified) const noexcept;

		std::pair<bool, std::string> resolve_history_references(std::string_view str, bool& modified) const;


		static int command_line_callback_st(ImGuiInputTextCallbackData * data) noexcept;

		int command_line_callback(ImGuiInputTextCallbackData * data) noexcept;

		static int try_push_style(ImGuiCol col, const std::optional<ImVec4>& color) {
			if (color) {
				ImGui::PushStyleColor(col, *color);
				return 1;
			}
			return 0;
		}

		static int try_push_style(ImGuiCol col, const std::optional<theme::constexpr_color>& color) {
			if (color) {
				ImGui::PushStyleColor(col, color->imv4());
				return 1;
			}
			return 0;
		}


		int is_space(std::string_view str) const;

		bool is_digit(char c) const;

		unsigned long get_length(std::string_view str) const;

		// Returns a vector containing each element that were space separated
		// Returns an empty optional if a '"' char was not matched with a closing '"',
		//                except if ignore_non_match was set to true
		std::optional<std::vector<std::string>> split_by_space(std::string_view in, bool ignore_non_match = false) const;

		inline void try_lock()
		{
			while (m_flag.test_and_set(std::memory_order_seq_cst)) {}
		}

		inline void try_unlock()
		{
			m_flag.clear(std::memory_order_seq_cst);
		}

		////////////

		value_type& m_argument_value;
		mutable std::shared_ptr<TerminalHelper> m_t_helper;

		bool m_should_show_next_frame{true};
		bool m_close_request{false};

		const char * const m_window_name;
		ImGuiWindowFlags m_flags{ImGuiWindowFlags_None};

		int m_base_width;
		int m_base_height;
		bool m_update_width{true};
		bool m_update_height{true};

		bool m_allow_x_resize{true};
		bool m_allow_y_resize{true};

		ImVec2 m_current_size{0.f, 0.f};

		struct theme m_colors{};

		// configuration
		bool m_autoscroll{true}; // TODO: accessors
		bool m_autowrap{true};  // TODO: accessors
		std::vector<std::string>::size_type m_last_size{0u};
		int m_level{message::severity::trace}; // TODO: accessors
#ifdef IMTERM_ENABLE_REGEX
		bool m_regex_search{true}; // TODO: accessors, button
#endif

		std::optional<std::string> m_autoscroll_text;
		std::optional<std::string> m_clear_text;
		std::optional<std::string> m_log_level_text;
		std::optional<std::string> m_autowrap_text;
		std::optional<std::string> m_filter_hint;
		std::string m_level_list_text{};
		const char* m_longest_log_level{nullptr}; // points to the longest log level, in m_level_list_text
		const char* m_lowest_log_level{nullptr}; // points to the lowest log level possible, in m_level_list_text
		message::severity::severity_t m_lowest_log_level_val{message::severity::trace};

		small_buffer_type m_log_text_filter_buffer{};
		small_buffer_type::size_type m_log_text_filter_buffer_usage{0u};


		// message panel variables
		unsigned long m_last_flush_at_history{0u}; // for the [-n] indicator on command line
		bool m_flush_bit{false};
		std::vector<message> m_logs{};
		std::vector<message>::size_type m_max_log_len{5'000}; // TODO: command
		std::vector<message>::size_type m_current_log_oldest_idx{0};


		// command line variables
		buffer_type m_command_buffer{};
		buffer_type::size_type m_buffer_usage{0u}; // max accessible: command_buffer[buffer_usage - 1]
		                                           // (buffer_usage might be 0 for empty string)
		buffer_type::size_type m_previous_buffer_usage{0u};
		bool m_should_take_focus{false};

		ImGuiID m_previously_active_id{0u};
		ImGuiID m_input_text_id{0u};

		// autocompletion
		std::vector<command_type_cref> m_current_autocomplete{};
		std::vector<std::string> m_current_autocomplete_strings{};
		std::string_view m_autocomlete_separator{" | "};
		position m_autocomplete_pos{position::down};
		bool m_command_entered{false};

		// command line: completion using history
		std::string m_command_line_backup{};
		std::string_view m_command_line_backup_prefix{};
		std::vector<std::string> m_command_history{};
		std::optional<std::vector<std::string>::iterator> m_current_history_selection{};

		bool m_ignore_next_textinput{false};
		bool m_has_focus{false};

		std::atomic_flag m_flag;
	};
}

#include "terminal.tpp"

#undef IMTERM_FMT_INCLUDED

#endif //IMTERM_TERMINAL_HPP
