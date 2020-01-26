#pragma once

#include "../../view/web/widget/File_viewer.h"
#include "../../common/util.h"
#include <Wt/WString.h>
#include <sstream>
#include <string_view>

class Explanation_writer;

void parse_explanation(string_view source,
                       Explanation_writer&,
                       File_viewer::Highlighter&);

class Explanation_writer
{
public:
    Explanation_writer() = default;

    virtual void plain_char(char) = 0;

    virtual void plain_string(std::string_view);

    void plain_range(const char* begin, const char* end);

    virtual int link(std::string_view) = 0;

    Wt::WString wstr() const;

protected:
    void raw_text_(char);
    void raw_text_(std::string_view);

private:
    std::ostringstream os_;
};

