#ifndef IMTERM_UTILS_HPP
#define IMTERM_UTILS_HPP

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

#include <string>
#include <string_view>
#include <array>
#include <optional>
#include <array>
#include <imgui.h>

#include "misc.hpp"

namespace ImTerm {
	// argument passed to commands
	template<typename Terminal>
	struct argument_t {
		using value_type = misc::non_void_t<typename Terminal::value_type>;

		value_type& val; // misc::details::structured_void if Terminal::value_type is void, reference to Terminal::value_type otherwise
		Terminal& term; // reference to the ImTerm::terminal that called the command

		std::vector<std::string> command_line; // list of arguments the user specified in the command line. command_line[0] is the command name
	};

	// structure used to represent a command
	template<typename Terminal>
	struct command_t {
		using command_function = void (*)(argument_t<Terminal>&);
		using further_completion_function = std::vector<std::string> (*)(argument_t<Terminal>& argument_line);

		std::string_view name{}; // name of the command
		std::string_view description{}; // short description
		command_function call{}; // function doing whatever you want

		further_completion_function complete{}; // function called when users starts typing in arguments for your command
		// return a vector of strings containing possible completions.

		friend constexpr bool operator<(const command_t& lhs, const command_t& rhs) {
			return lhs.name < rhs.name;
		}

		friend constexpr bool operator<(const command_t& lhs, const std::string_view& rhs) {
			return lhs.name < rhs;
		}

		friend constexpr bool operator<(const std::string_view& lhs, const command_t& rhs) {
			return lhs < rhs.name;
		}
	};

	struct message {
		enum class type {
			user_input,             // terminal wants to log user input
			error,                  // terminal wants to log an error in user input
			cmd_history_completion, // terminal wants to log that it replaced "!:*" family input in the appropriate string
		};
		struct severity {
			enum severity_t { // done this way to be used as array index without a cast
				trace,
				debug,
				info,
				warn,
				err,
				critical
			};
		};

		severity::severity_t severity; // severity of the message
		std::string value; // text to be displayed

		// the actual color used depends on the message's severity
		size_t color_beg; // color begins at value.data() + color_beg
		size_t color_end; // color ends at value.data() + color_end - 1
		// if color_beg == color_end, nothing is colorized.

		bool is_term_message; // if set to true, current msg is considered to be originated from the terminal,
		// never filtered out by severity filter, and applying for different rules regarding colors.
		// severity is also ignored for such messages
	};

	enum class config_panels {
		autoscroll,
		autowrap,
		clearbutton,
		filter,
		long_filter, // like filter, but takes up space
		loglevel,
		blank, // invisible panel that takes up place, aligning items to the right. More than one can be used, splitting up the consumed space
		// ie: {clearbutton (C), blank, filter (F), blank, loglevel (L)} will result in the layout [C           F           L]
		// Shares space with long_filter
				none
	};

	enum class position {
		up,
		down,
		nowhere // disabled
	};

	// Various settable colors for the terminal
	// if an optional is empty, current ImGui color will be used
	struct theme {
		struct constexpr_color {
			float r,g,b,a;

			ImVec4 imv4() const {
				return {r,g,b,a};
			}
		};

		std::string_view name; // if you want to give a name to the theme

		std::optional<constexpr_color> text;                        // global text color
		std::optional<constexpr_color> window_bg;                   // ImGuiCol_WindowBg & ImGuiCol_ChildBg
		std::optional<constexpr_color> border;                      // ImGuiCol_Border
		std::optional<constexpr_color> border_shadow;               // ImGuiCol_BorderShadow
		std::optional<constexpr_color> button;                      // ImGuiCol_Button
		std::optional<constexpr_color> button_hovered;              // ImGuiCol_ButtonHovered
		std::optional<constexpr_color> button_active;               // ImGuiCol_ButtonActive
		std::optional<constexpr_color> frame_bg;                    // ImGuiCol_FrameBg
		std::optional<constexpr_color> frame_bg_hovered;            // ImGuiCol_FrameBgHovered
		std::optional<constexpr_color> frame_bg_active;             // ImGuiCol_FrameBgActive
		std::optional<constexpr_color> text_selected_bg;            // ImGuiCol_TextSelectedBg, for text input field
		std::optional<constexpr_color> check_mark;                  // ImGuiCol_CheckMark
		std::optional<constexpr_color> title_bg;                    // ImGuiCol_TitleBg
		std::optional<constexpr_color> title_bg_active;             // ImGuiCol_TitleBgActive
		std::optional<constexpr_color> title_bg_collapsed;          // ImGuiCol_TitleBgCollapsed
		std::optional<constexpr_color> message_panel;               // logging panel
		std::optional<constexpr_color> auto_complete_selected;      // left-most text in the autocompletion OSD
		std::optional<constexpr_color> auto_complete_non_selected;  // every text but the left most in the autocompletion OSD
		std::optional<constexpr_color> auto_complete_separator;     // color for the separator in the autocompletion OSD
		std::optional<constexpr_color> cmd_backlog;                 // color for message type user_input
		std::optional<constexpr_color> cmd_history_completed;       // color for message type cmd_history_completion
		std::optional<constexpr_color> log_level_drop_down_list_bg; // ImGuiCol_PopupBg
		std::optional<constexpr_color> log_level_active;            // ImGuiCol_HeaderActive
		std::optional<constexpr_color> log_level_hovered;           // ImGuiCol_HeaderHovered
		std::optional<constexpr_color> log_level_selected;          // ImGuiCol_Header
		std::optional<constexpr_color> scrollbar_bg;                // ImGuiCol_ScrollbarBg
		std::optional<constexpr_color> scrollbar_grab;              // ImGuiCol_ScrollbarGrab
		std::optional<constexpr_color> scrollbar_grab_active;       // ImGuiCol_ScrollbarGrabActive
		std::optional<constexpr_color> scrollbar_grab_hovered;      // ImGuiCol_ScrollbarGrabHovered
		std::optional<constexpr_color> filter_hint;                 // ImGuiCol_TextDisabled
		std::optional<constexpr_color> filter_text;                 // user input in log filter
		std::optional<constexpr_color> matching_text;               // text matching the log filter

		std::array<std::optional<constexpr_color>, message::severity::critical + 1> log_level_colors{}; // colors by severity
	};


	namespace themes {

		constexpr theme light = {
				"Light Rainbow",
				theme::constexpr_color{0.100f, 0.100f, 0.100f, 1.000f}, //text
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 1.000f}, //window_bg
				theme::constexpr_color{0.600f, 0.600f, 0.600f, 1.000f}, //border
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //border_shadow
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //button
				theme::constexpr_color{0.824f, 0.765f, 0.765f, 0.875f}, //button_hovered
				theme::constexpr_color{0.627f, 0.569f, 0.569f, 0.875f}, //button_active
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //frame_bg
				theme::constexpr_color{0.824f, 0.765f, 0.765f, 0.875f}, //frame_bg_hovered
				theme::constexpr_color{0.627f, 0.569f, 0.569f, 0.875f}, //frame_bg_active
				theme::constexpr_color{0.260f, 0.590f, 0.980f, 0.350f}, //text_selected_bg
				theme::constexpr_color{0.843f, 0.000f, 0.373f, 1.000f}, //check_mark
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 0.850f}, //title_bg
				theme::constexpr_color{0.165f, 0.365f, 0.506f, 1.000f}, //title_bg_active
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 0.850f}, //title_collapsed
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //message_panel
				theme::constexpr_color{0.196f, 1.000f, 0.196f, 1.000f}, //auto_complete_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 1.000f}, //auto_complete_non_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.392f}, //auto_complete_separator
				theme::constexpr_color{0.519f, 0.118f, 0.715f, 1.000f}, //cmd_backlog
				theme::constexpr_color{1.000f, 0.430f, 0.059f, 1.000f}, //cmd_history_completed
				theme::constexpr_color{0.901f, 0.843f, 0.843f, 0.784f}, //log_level_drop_down_list_bg
				theme::constexpr_color{0.443f, 0.705f, 1.000f, 1.000f}, //log_level_active
				theme::constexpr_color{0.443f, 0.705f, 0.784f, 0.705f}, //log_level_hovered
				theme::constexpr_color{0.443f, 0.623f, 0.949f, 1.000f}, //log_level_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //scrollbar_bg
				theme::constexpr_color{0.470f, 0.470f, 0.588f, 1.000f}, //scrollbar_grab
				theme::constexpr_color{0.392f, 0.392f, 0.509f, 1.000f}, //scrollbar_grab_active
				theme::constexpr_color{0.509f, 0.509f, 0.666f, 1.000f}, //scrollbar_grab_hovered
				theme::constexpr_color{0.470f, 0.470f, 0.470f, 1.000f}, //filter_hint
				theme::constexpr_color{0.100f, 0.100f, 0.100f, 1.000f}, //filter_text
				theme::constexpr_color{0.549f, 0.196f, 0.039f, 1.000f}, //matching_text
				{
						theme::constexpr_color{0.078f, 0.117f, 0.764f, 1.f}, // log_level::trace
						theme::constexpr_color{0.100f, 0.100f, 0.100f, 1.f}, // log_level::debug
						theme::constexpr_color{0.301f, 0.529f, 0.000f, 1.f}, // log_level::info
						theme::constexpr_color{0.784f, 0.431f, 0.058f, 1.f}, // log_level::warning
						theme::constexpr_color{0.901f, 0.117f, 0.117f, 1.f}, // log_level::error
						theme::constexpr_color{0.901f, 0.117f, 0.117f, 1.f}, // log_level::critical
				}
		};

		constexpr theme cherry {
				"Dark Cherry",
				theme::constexpr_color{0.649f, 0.661f, 0.669f, 1.000f}, //text
				theme::constexpr_color{0.130f, 0.140f, 0.170f, 1.000f}, //window_bg
				theme::constexpr_color{0.310f, 0.310f, 1.000f, 0.000f}, //border
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //border_shadow
				theme::constexpr_color{0.470f, 0.770f, 0.830f, 0.140f}, //button
				theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.860f}, //button_hovered
				theme::constexpr_color{0.455f, 0.198f, 0.301f, 1.000f}, //button_active
				theme::constexpr_color{0.200f, 0.220f, 0.270f, 1.000f}, //frame_bg
				theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.780f}, //frame_bg_hovered
				theme::constexpr_color{0.455f, 0.198f, 0.301f, 1.000f}, //frame_bg_active
				theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.430f}, //text_selected_bg
				theme::constexpr_color{0.710f, 0.202f, 0.207f, 1.000f}, //check_mark
				theme::constexpr_color{0.232f, 0.201f, 0.271f, 1.000f}, //title_bg
				theme::constexpr_color{0.502f, 0.075f, 0.256f, 1.000f}, //title_bg_active
				theme::constexpr_color{0.200f, 0.220f, 0.270f, 0.750f}, //title_bg_collapsed
				theme::constexpr_color{0.100f, 0.100f, 0.100f, 0.500f}, //message_panel
				theme::constexpr_color{1.000f, 1.000f, 1.000f, 1.000f}, //auto_complete_selected
				theme::constexpr_color{0.500f, 0.450f, 0.450f, 1.000f}, //auto_complete_non_selected
				theme::constexpr_color{0.600f, 0.600f, 0.600f, 1.000f}, //auto_complete_separator
				theme::constexpr_color{0.860f, 0.930f, 0.890f, 1.000f}, //cmd_backlog
				theme::constexpr_color{0.153f, 0.596f, 0.498f, 1.000f}, //cmd_history_completed
				theme::constexpr_color{0.100f, 0.100f, 0.100f, 0.860f}, //log_level_drop_down_list_bg
				theme::constexpr_color{0.730f, 0.130f, 0.370f, 1.000f}, //log_level_active
				theme::constexpr_color{0.450f, 0.190f, 0.300f, 0.430f}, //log_level_hovered
				theme::constexpr_color{0.730f, 0.130f, 0.370f, 0.580f}, //log_level_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //scrollbar_bg
				theme::constexpr_color{0.690f, 0.690f, 0.690f, 0.800f}, //scrollbar_grab
				theme::constexpr_color{0.490f, 0.490f, 0.490f, 0.800f}, //scrollbar_grab_active
				theme::constexpr_color{0.490f, 0.490f, 0.490f, 1.000f}, //scrollbar_grab_hovered
				theme::constexpr_color{0.649f, 0.661f, 0.669f, 1.000f}, //filter_hint
				theme::constexpr_color{1.000f, 1.000f, 1.000f, 1.000f}, //filter_text
				theme::constexpr_color{0.490f, 0.240f, 1.000f, 1.000f}, //matching_text
				{
						theme::constexpr_color{0.549f, 0.561f, 0.569f, 1.f}, // log_level::trace
						theme::constexpr_color{0.153f, 0.596f, 0.498f, 1.f}, // log_level::debug
						theme::constexpr_color{0.459f, 0.686f, 0.129f, 1.f}, // log_level::info
						theme::constexpr_color{0.839f, 0.749f, 0.333f, 1.f}, // log_level::warning
						theme::constexpr_color{1.000f, 0.420f, 0.408f, 1.f}, // log_level::error
						theme::constexpr_color{1.000f, 0.420f, 0.408f, 1.f}, // log_level::critical
				},
		};

		constexpr std::array list {
				cherry,
				light
		};
	}
}



#endif //IMTERM_UTILS_HPP
