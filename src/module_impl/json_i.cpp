module;
#include <string>
#include <memory>
#include <functional>
#include <clocale>

module lsf;

namespace lsf
{

    Json::Json()
    {
        // BUG 需要重新初始化这块
        // // 输入抽象
        buff_ = std::make_shared<lsf::FilterBuff>();

        // 创建词法分析器
        lexer_ = std::make_shared<lsf::Lexer>(buff_);

        /************************/
        wrap_lexer_ = std::make_shared<FunnyTokenGen>(lexer_, buff_);

        // 创建语法分析器
        parser_ = std::make_unique<lsf::JsonParser>(wrap_lexer_);

        // 节点构建器
        builder = std::make_shared<TreeBuilder>();

        parser_->set_builder(builder);
    }

    // https://en.cppreference.com/w/cpp/memory/unique_ptr
    // https://www.cnblogs.com/misteo/p/14062426.html
    Json::~Json()
    {
        builder->dealloc_node();
    }

    std::string lexer_messages(Location stat_for_rc, Token lex_token)
    {
        std::stringstream s{};
        s << "  " << lsf::tokentype_to_string(lex_token.type_) << " : " << to_cstring(lex_token.value_)
          << "\n位于:" << stat_for_rc.line_ << "行," << stat_for_rc.column_ << "列";
        return s.str();
    }

    bool Json::run(std::function<void(ErrorType, const std::string &)> f)
    {
        error_msg_.clear();
        try
        {
            if (!parser_->parser())
            {
                error_msg_ += "语法解析出错,当前期待以下词法单元:\n";
                error_msg_ += lsf::parser_messages(parser_->get_expect_token());
                error_msg_ += "当前词法单元是:\n";
                error_msg_ += lsf::lexer_messages(wrap_lexer_->token_position(), lexer_->get_token());
                f(ErrorType::ParserError, error_msg_);
                return false;
            }
        }
        catch (const lsf::LexerError &e)
        {
            error_msg_ += "词法解析出错,当前期待以下词法单元";
            error_msg_ += lsf::lexer_messages(wrap_lexer_->token_position(), lexer_->get_token());
            f(ErrorType::LexError, error_msg_);
            return false;
        }
        catch (const std::runtime_error &e)
        {
            error_msg_ += e.what();
            f(ErrorType::UnknowError, error_msg_);
            return false;
        }
        return true;
    }

    bool Json::weak_type_check(std::function<void(ErrorType et, const std::string &message)> f)
    {
        // TODO 修改环境
        LocaleGuard lg;
        error_msg_.clear();
        lsf::WeakTypeChecker typer;
        if (!typer.check_type(builder->get_ast()))
        {
            error_msg_ += "类型检查失败\n";
            error_msg_ += typer.get_error().first;
            error_msg_ += to_cstring(typer.get_error().second);
            error_msg_ += '\n';
            f(ErrorType::WeakTypeCheckError, error_msg_);
            return false;
        }
        return true;
    }

    std::string Json::get_errors() const
    {
        return error_msg_;
    }

}
