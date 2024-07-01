#include "SourceInfo.h"

ast::SourceInfo::SourceInfo(size_t line_no, size_t col_no)
        :
        line_no(line_no),
        col_no(col_no) {}

ast::SourceInfo::SourceInfo() :
        line_no(0),
        col_no(0) {

}
