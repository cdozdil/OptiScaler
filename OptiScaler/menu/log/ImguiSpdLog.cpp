#include "ImguiSpdLog.h"

class SinkLineContent
{
  public:
    spdlog::level::level_enum LogLevel; // If n_levels, the message pushed counts to the previous pushed line

    int32_t BeginIndex; // Base offset into the text buffer

    struct ColorDataRanges
    {
        uint32_t SubStringBegin : 12;
        uint32_t SubStringEnd : 12;
        uint32_t FormatTag : 8;
    };

    ImVector<ColorDataRanges> FormattedStringRanges;
};

// TODO: work on filters, they are really needed, did optimize the memory storage quite a bit tho
class ImGuiSpdLogAdaptor : public spdlog::sinks::base_sink<std::mutex>
{

    using sink_t = spdlog::sinks::base_sink<std::mutex>;

  public:
    void DrawLogWindow()
    {

        if (ImGui::Begin("CyberXeSS Log", &ShowWindow))
        {

            // Options submenu menu
            if (ImGui::BeginPopup("Options"))
            {

                ImGui::Checkbox("Auto-scroll", &EnableAutoScrolling);
                ImGui::EndPopup();
            }

            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");

            ImGui::SameLine();
            ImGui::SameLine();

            if (ImGui::Button("Clear"))
                ClearLogBuffers();

            ImGui::SameLine();

            ImGui::Text("%d messages logged, using %dmb memory", NumberOfLogEntries,
                        (LoggedContent.size() + IndicesInBytes) / (1024 * 1024));

            // Filter out physical messgaes through logger
            static const char* LogLevels[] = SPDLOG_LEVEL_NAMES;

            static const auto LogSelectionWidth = []() -> float
            {
                float LongestTextWidth = 0;
                for (auto LogLevelText : LogLevels)
                {

                    auto TextWidth = ImGui::CalcTextSize(LogLevelText).x;
                    if (TextWidth > LongestTextWidth)
                        LongestTextWidth = TextWidth;
                }

                return LongestTextWidth + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetFrameHeight();
            }();

            auto ComboBoxRightAlignment =
                ImGui::GetWindowSize().x - (LogSelectionWidth + ImGui::GetStyle().WindowPadding.x);
            auto ActiveLogLevel = spdlog::get_level();

            ImGui::SetNextItemWidth(LogSelectionWidth);
            ImGui::SameLine(ComboBoxRightAlignment);

            ImGui::Combo("##ActiveLogLevel", reinterpret_cast<int32_t*>(&ActiveLogLevel), LogLevels,
                         sizeof(LogLevels) / sizeof(LogLevels[0]));
            spdlog::set_level(ActiveLogLevel);

            // Filter out messages on display
            FilterTextMatch.Draw("##LogFilter",
                                 ImGui::GetWindowSize().x - (LogSelectionWidth + ImGui::GetStyle().WindowPadding.x * 2 +
                                                             ImGui::GetStyle().FramePadding.x));

            ImGui::SetNextItemWidth(LogSelectionWidth);
            ImGui::SameLine(ComboBoxRightAlignment);
            ImGui::Combo("##FilterLogLevel", &FilterLogLevel, LogLevels, sizeof(LogLevels) / sizeof(LogLevels[0]));

            // Draw main log window
            ImGui::Separator();
            ImGui::BeginChild("LogTextView", ImVec2(0, 0), false,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const std::lock_guard LogLock(sink_t::mutex_);

            RebuildFilterWithPreviousStates();

            ImGuiListClipper ViewClipper;
            ViewClipper.Begin(FilteredView.size());

            while (ViewClipper.Step())
            {
                int32_t StylesPushedToStack = 0;
                for (auto ClipperLineNumber = ViewClipper.DisplayStart; ClipperLineNumber < ViewClipper.DisplayEnd;
                     ++ClipperLineNumber)
                {
                    auto& LogMetaDataEntry = LogMetaData[FilteredView[ClipperLineNumber]];

                    if (LogMetaDataEntry.LogLevel == spdlog::level::n_levels)
                        ImGui::Indent();

                    for (auto i = 0; i < LogMetaDataEntry.FormattedStringRanges.size(); ++i)
                    {
                        static const ImVec4 BrightColorsToVec[] {
                            { 0, 0, 0, 1 }, // COLOR_BRIGHTBLACK
                            { 1, 0, 0, 1 }, // COLOR_BRIGHTRED
                            { 0, 1, 0, 1 }, // COLOR_BRIGHTGREEN
                            { 1, 1, 0, 1 }, // COLOR_BRIGHTYELLOW
                            { 0, 0, 1, 1 }, // COLOR_BRIGHTBLUE
                            { 1, 0, 1, 1 }, // COLOR_BRIGHTMAGENTA
                            { 0, 1, 1, 1 }, // COLOR_BRIGHTCYAN
                            { 1, 1, 1, 1 }  // COLOR_BRIGHTWHITE
                        };

                        ImGui::PushStyleColor(ImGuiCol_Text,
                                              BrightColorsToVec[LogMetaDataEntry.FormattedStringRanges[i].FormatTag]);
                        ++StylesPushedToStack;

                        auto FormatRangeBegin = LoggedContent.begin() + LogMetaDataEntry.BeginIndex +
                                                LogMetaDataEntry.FormattedStringRanges[i].SubStringBegin;
                        auto FormatRangeEnd = LoggedContent.begin() + LogMetaDataEntry.BeginIndex +
                                              LogMetaDataEntry.FormattedStringRanges[i].SubStringEnd;
                        ImGui::TextUnformatted(FormatRangeBegin, FormatRangeEnd);

                        if (LogMetaDataEntry.FormattedStringRanges.size() - (i + 1))
                            ImGui::SameLine();
                    }

                    if (LogMetaDataEntry.LogLevel == spdlog::level::n_levels)
                        ImGui::Unindent();
                }

                ImGui::PopStyleColor(StylesPushedToStack);
            }

            ViewClipper.End();
            ImGui::PopStyleVar();

            if (EnableAutoScrolling && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
        }

        ImGui::End();
    }

    void ClearLogBuffers(IN bool DisableLock = false)
    {
        if (!DisableLock)
            sink_t::mutex_.lock();

        LoggedContent.clear();
        LogMetaData.clear();
        NumberOfLogEntries = 0;
        IndicesInBytes = 0;

        if (!DisableLock)
            sink_t::mutex_.unlock();
    }

  protected:
    // Writing version 2, this will accept ansi escape sequences for colors
    void sink_it_(IN const spdlog::details::log_msg& LogMessage) final
    {
        // This is all protected by the base sink under a mutex
        ++NumberOfLogEntries;

        // Format the logged message and push it into the text buffer
        spdlog::memory_buf_t FormattedBuffer;

        sink_t::formatter_->format(LogMessage, FormattedBuffer);

        std::string FormattedText = FormattedBuffer;

        // Process string by converting the color range passed to an escape sequence first,
        // yes may not be the nicest way of doing this but its way easier to process later.
        const char* ColorToEscapeSequence[] { ESC_BRIGHTMAGENTA, ESC_CYAN,      ESC_BRIGHTGREEN,
                                              ESC_BRIGHTYELLOW,  ESC_BRIGHTRED, ESC_RED };

        FormattedText.insert(LogMessage.color_range_start, ColorToEscapeSequence[LogMessage.level]);
        FormattedText.insert(LogMessage.color_range_end + 5, "\x1b[0m");

        bool FilterPassing = LogMessage.level >= FilterLogLevel;
        FilterPassing &=
            FilterTextMatch.PassFilter(FormattedText.c_str(), FormattedText.c_str() + FormattedText.size());

        // Parse formatted logged string for ansi escape sequences
        auto OldTextBufferSize = LoggedContent.size();
        SinkLineContent MessageData2 { LogMessage.level, OldTextBufferSize };

        AnsiEscapeSequenceTag LastSequenceTagSinceBegin = FORMAT_RESET_COLORS;

        // Prematurely filter out immediately starting non default formats,
        // and then enter the main processing loop
        switch (FormattedText[0])
        {
        case '\x1b':
        case '\n':
            break;

        default:

            SinkLineContent::ColorDataRanges FormatPush { 0, 0, LastSequenceTagSinceBegin };
            MessageData2.FormattedStringRanges.push_back(FormatPush);
            break;
        }

        for (auto i = 0; i < FormattedText.size(); ++i)
        {
            switch (FormattedText[i])
            {
            case '\n':
            {

                // Handle new line bullshit, spdlog will terminate any logged message witha  new line
                // we can also assume this may not be the last line, so we continue the previous sequence into
                // the next logically text line if necessary and reconfigure pushstate
                if (MessageData2.FormattedStringRanges.size())
                    MessageData2.FormattedStringRanges.back().SubStringEnd = i;
                if (FilterPassing)
                    FilteredView.push_back(LogMetaData.size());
                LogMetaData.push_back(MessageData2);

                IndicesInBytes += MessageData2.FormattedStringRanges.size() * sizeof(SinkLineContent::ColorDataRanges) +
                                  sizeof(SinkLineContent);
                MessageData2.LogLevel = spdlog::level::n_levels;
                MessageData2.BeginIndex = OldTextBufferSize + i + 1;
                MessageData2.FormattedStringRanges.clear();

                // Continue previous escape sequences pushed in the previous line
                SinkLineContent::ColorDataRanges FormatPush { i + 1, 0, LastSequenceTagSinceBegin };
                MessageData2.FormattedStringRanges.push_back(FormatPush);
            }
            break;
            case '\x1b':
            {

                // Handle ansi escape sequence, convert textual to operand
                if (FormattedText[i + 1] != '[')
                    throw std::runtime_error("Invalid ansi escape sequence passed");

                size_t PositionProcessed = 0;
                auto EscapeSequenceCode = static_cast<AnsiEscapeSequenceTag>(
                    std::stoi(&FormattedText[i + 2],
                              &PositionProcessed)); // this may throw, in which case we let it pass down

                if (FormattedText[i + 2 + PositionProcessed] != 'm')
                    throw std::runtime_error("Invalid ansi escape sequence operand was passed");
                ++PositionProcessed;
                LastSequenceTagSinceBegin = EscapeSequenceCode;

                SinkLineContent::ColorDataRanges FormatPush { i, 0, EscapeSequenceCode };
                if (MessageData2.FormattedStringRanges.size())
                    MessageData2.FormattedStringRanges.back().SubStringEnd = FormatPush.SubStringBegin;
                MessageData2.FormattedStringRanges.push_back(FormatPush);

                // Now the escape code has to be removed from the string,
                // the iterator has to be kept stable, otherwise the next round could skip a char
                FormattedText.erase(FormattedText.begin() + i--, FormattedText.begin() + (i + 2 + PositionProcessed));
            }
            }
        }

        // Append processed string to log text buffer
        LoggedContent.append(FormattedText.c_str(), FormattedText.c_str() + FormattedText.size());
    }

    void flush_() {}

  private:
    // TODO: Need to implement double buffering for the filter
    void RebuildFilterWithPreviousStates()
    {
        int32_t RebuildType = PreviousFilterLevel != FilterLogLevel;

        RebuildType |= memcmp(PreviousFilterText, FilterTextMatch.InputBuf, sizeof(PreviousFilterText));

        if (RebuildType)
        {
            // Filter was completely changed, have to rebuild array (very expensive,
            //	this may result in short freezes or stuttering on really large logs,
            //	one way to solve this could be to defer the calculation of the filter view
            //	to a thread pool and use a double buffering mechanism)

            auto NewLinePasses = false;
            FilteredView.clear();

            for (auto i = 0; i < LogMetaData.size(); ++i)
            {

                if (LogMetaData[i].LogLevel == spdlog::level::n_levels&&)
                {
                    FilteredView.push_back(i);
                    continue;
                }

                if (LogMetaData[i].LogLevel < FilterLogLevel)
                {
                    NewLinePasses = false;
                    continue;
                }

                auto LineBegin = LoggedContent.begin() + LogMetaData[i].BeginIndex +
                                 LogMetaData[i].FormattedStringRanges.front().SubStringBegin;
                auto LineEnd = LoggedContent.begin() + LogMetaData[i].BeginIndex +
                               LogMetaData[i].FormattedStringRanges.back().SubStringEnd;

                if (!FilterTextMatch.PassFilter(LineBegin, LineEnd))
                {
                    NewLinePasses = false;
                    continue;
                }

                NewLinePasses = true;
                FilteredView.push_back(i);
            }
        }

        PreviousFilterLevel = FilterLogLevel;
        memcpy(PreviousFilterText, FilterTextMatch.InputBuf, sizeof(PreviousFilterText));
    }

    // Using faster more efficient replacements of stl types for rendering
    ImGuiTextBuffer LoggedContent;

    // Cannot use ImVetctor here as the type is not trivially copyable
    std::vector<SinkLineContent> LogMetaData;

    // the type has to be moved into, slightly more expensive
    // but overall totally fine, at least no weird hacks
    ImGuiTextFilter FilterTextMatch;

    // A filtered array of indexes into the LogMetaData vector
    ImVector<int32_t> FilteredView;

    // this view is calculated once any filter changes
    int32_t FilterLogLevel = spdlog::level::trace;

    // Counts the number of entries logged
    uint32_t NumberOfLogEntries = 0;

    // Keeps track of the amount of memory allocated by indices
    uint32_t IndicesInBytes = 0;
    bool EnableAutoScrolling = true;

    // Previous Frame's filterdata, if any of these change to the new values the filter has to be recalculated
    int32_t PreviousFilterLevel = spdlog::level::trace;
    decltype(ImGuiTextFilter::InputBuf) PreviousFilterText {};
};
