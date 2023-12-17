module;
#include <memory>
#include <vector>
#include <cassert>
#include <limits>
#include <stack>
#include <sstream>

export module lsf:jsonparser;

import :constant;
import :parser_tree;
import :lexer;

namespace lsf
{
    class JsonParser
    {
    public:
        JsonParser(std::shared_ptr<GenToken> gen);
        void set_builder(std::shared_ptr<ParserResultBuilder> b);
        [[nodiscard]] bool parser();
        const std::vector<lsf::Type> &get_expect_token() const;

    private:
        using TType = lsf::Type;
        bool json();
        bool element();
        bool value();
        bool obj();
        bool mb_ws();
        bool mb_ws_r();
        bool memberL();
        bool member();
        bool array();
        bool arr_ws();
        bool arr_ws_r();
        bool elementsL();
        bool unuse();

        bool isTerminator(TType type);

    private:
        std::shared_ptr<GenToken> gen_;
        std::shared_ptr<ParserResultBuilder> builder_;
        std::vector<lsf::Type> expect_array_;
    };

    std::string parser_messages(Location stat_for_rc, Token lex_token, std::vector<Type> expects)
    {
        std::stringstream s{};
        s << "当前语法期待以下词法单元:\n";
        for (const auto &i : expects)
            s << "  " << tokentype_to_string(i) << '\n';
        s << lexer_messages(stat_for_rc, lex_token) << '\n';
        return s.str();
    }

    // 把递归下降转换成循环
    class R_JsonParser
    {
    public:
        R_JsonParser(std::shared_ptr<GenToken> gen);
        void set_builder(std::shared_ptr<ParserResultBuilder> b);
        [[nodiscard]] bool parser();
        const std::vector<lsf::Type> &get_expect_token() const;

    private:
        using TType = lsf::Type;
        bool json();
        /// 不建议对此函数有任何想法,因为全是复杂的令人难过的转换逻辑,
        bool value();
        bool unuse();

        bool isTerminator(TType type);

    private:
        std::shared_ptr<GenToken> gen_;
        std::shared_ptr<ParserResultBuilder> builder_;
        std::vector<lsf::Type> expect_array_;
    };

    // namespace end
}

namespace lsf
{

    JsonParser::JsonParser(std::shared_ptr<GenToken> gen) : gen_(gen),
                                                            expect_array_{}
    {
    }

    void JsonParser::set_builder(std::shared_ptr<ParserResultBuilder> b)
    {
        builder_ = b;
    }

    bool JsonParser::parser()
    {
        assert(builder_ && gen_);
        builder_->before_build();
        expect_array_.clear(); // 之后push_back或assign都一样

        gen_->next_();
        return json();
    }

    const std::vector<Type> &JsonParser::get_expect_token() const
    {
        return expect_array_;
    }

    //  json -> element
    //  json.node=element.node
    bool JsonParser::json()
    {
        if (element())
        {
            if (isTerminator(TType::END))
            {
                builder_->after_build();
                return true;
            }
            expect_array_.push_back({TType::END});
        }
        return false;
    }

    // element -> unuse value unuse
    // element.node=value.node
    bool JsonParser::element()
    {
        unuse();
        if (value())
            return unuse();
        return false;
    }

    // value -> obj | array | String | Number | KeyWord | Null
    // value.node=obj.node,array.node
    // value.node=new Jnode<(string|number|keyword|Null)>;
    bool JsonParser::value()
    {
        switch (gen_->current_().type_)
        {
        case Type::LBRACE:
            return obj();
        case Type::LSQUARE:
            return array();
        case Type::String:
        {
            builder_->build_string(gen_->current_().value_);
            gen_->next_();
            return true;
        }
        case Type::Number:
        {
            builder_->build_number(gen_->current_().value_);
            gen_->next_();
            return true;
        }
        case Type::KeyWord:
        {
            builder_->build_keyword(gen_->current_().value_);
            gen_->next_();
            return true;
        }
        case Type::Null:
        {
            builder_->build_Null(gen_->current_().value_);
            gen_->next_();
            return true;
        }
        default:
            expect_array_.assign({Type::String, Type::Number, Type::KeyWord, Type::Null});
            return false;
        }
    }

    // obj -> '{' mb_ws '}'
    // obj.node=new Jnode<obj>
    // obj.node.left=mb_ws.node
    // obj.node.right=obj.node
    bool JsonParser::obj()
    {
        if (isTerminator(TType::LBRACE))
        {
            gen_->next_();
            if (mb_ws())
            {
                if (isTerminator(TType::RBRACE))
                {
                    gen_->next_();
                    builder_->build_obj();
                    return true;
                }
                expect_array_.push_back(TType::RBRACE);
                return false;
            }
            return false;
        }
        expect_array_.push_back(TType::LBRACE);
        return false;
    }

    // mb_ws -> unuse mb_ws_r
    // mb_ws.node=mb_ws_r.node
    bool JsonParser::mb_ws()
    {
        unuse();
        return mb_ws_r();
    }

    // mb_ws_r -> String unuse ':' element memberL | e
    // element.node.key=string
    // memberL.node=element.node+memberL.node
    // mb_ws_r.node=memberL.node
    bool JsonParser::mb_ws_r()
    {
        if (isTerminator(TType::String))
        {
            auto key = std::move(gen_->current_().value_);
            gen_->next_();
            unuse();
            if (isTerminator(TType::COLON))
            {
                gen_->next_();
                if (element())
                {
                    builder_->set_memberkey(key);
                    builder_->can_start_iteration();
                    return memberL();
                }
                else
                    return false;
            }
            expect_array_.push_back(TType::COLON);
            return false;
        }
        else if (isTerminator(TType::RBRACE))
        {
            //        root_ = null_;
            builder_->build_null_mbr();
            return true;
        }
        else
        {
            expect_array_.push_back(TType::String);
            expect_array_.push_back(TType::RBRACE);
            return false;
        }
    }

    // memberL -> ',' member memberL | e
    // memberL.node=member.node+memberL.node
    bool JsonParser::memberL()
    {
        if (isTerminator(TType::RBRACE))
        {
            return true;
        }
        //    builder_->start_iteration();
        while (isTerminator(TType::COMMA))
        {
            gen_->next_();
            if (member())
            {
                builder_->move_next();
                if (isTerminator(TType::RBRACE))
                {
                    builder_->finish_iteration();
                    return true;
                }
                continue;
            }
            return false;
        }
        expect_array_.push_back(TType::RBRACE);
        expect_array_.push_back(TType::COMMA);
        return false;
    }

    // member -> unuse String unuse ':' element
    // member.node=element.node
    // member.node.key=String
    bool JsonParser::member()
    {
        unuse();
        if (isTerminator(TType::String))
        {
            auto key = std::move(gen_->current_().value_);
            gen_->next_();
            unuse();
            if (isTerminator(TType::COLON))
            {
                gen_->next_();
                if (element())
                {
                    builder_->set_memberkey(key);
                    return true;
                }
                return false;
            }
            expect_array_.push_back(TType::COLON);
            return false;
        }
        expect_array_.push_back(TType::String);
        return false;
    }

    // array -> '[' arr_ws ']'
    // array.node=new Jnode<arr>
    // array.node.right=array.node
    // array.node.left=arr_ws.node
    bool JsonParser::array()
    {
        if (isTerminator(TType::LSQUARE))
        {
            gen_->next_();
            if (arr_ws())
            {
                if (isTerminator(TType::RSQUARE))
                {
                    gen_->next_();
                    builder_->build_arr();
                    return true;
                }
                expect_array_.push_back(TType::RSQUARE);
                return false;
            }
            return false;
        }
        expect_array_.push_back(TType::LSQUARE);
        return false;
    }

    // arr_ws -> unuse arr_ws_r
    // arr_ws.node=arr_ws_r.node
    bool JsonParser::arr_ws()
    {
        unuse();
        return arr_ws_r();
    }

    // arr_ws_r -> value unuse elementsL | e
    // arr_ws_r.node=value.node+elementsL.node
    bool JsonParser::arr_ws_r()
    {
        //    switch (gen_->current_().type_) {
        //    case TType::LBRACE:
        //    case TType::LSQUARE:
        //    case TType::String:
        //    case TType::Number:
        //    case TType::KeyWord:
        //    case TType::Null:{
        //        if(value()){
        //            builder_->can_start_iteration();
        //            unuse();
        //            return elementsL();
        //        }
        //        return false;
        //    }
        //    case TType::RSQUARE:
        ////        root_ = null_;
        //        builder_->build_null_mbr();
        //        return true;
        //    default:
        //        expect_array_.assign({TType::RSQUARE});
        //        return false;
        //    }
        if (gen_->current_().type_ == TType::RSQUARE)
        {
            builder_->build_null_mbr();
            return true;
        }
        else
        {
            if (value())
            {
                builder_->can_start_iteration();
                unuse();
                return elementsL();
            }
            expect_array_.push_back({TType::RSQUARE});
            return false;
        }
    }

    // elementsL -> ',' element elementsL | e
    // elementsL.node=element.node+elementsL.node
    bool JsonParser::elementsL()
    {
        if (isTerminator(Type::RSQUARE))
            return true;
        //    builder_->start_iteration();
        while (isTerminator(Type::COMMA))
        {
            gen_->next_();
            if (element())
            {
                builder_->move_next();
                if (isTerminator(Type::RSQUARE))
                {
                    builder_->finish_iteration();
                    return true;
                }
                continue;
            }
            return false;
        }
        expect_array_.push_back(Type::RSQUARE);
        expect_array_.push_back(Type::COMMA);
        return false;
    }

    // unuse -> wc unuse | e
    // wc -> WhiteSpace | Comment
    bool JsonParser::unuse()
    {
        while (isTerminator(TType::WHITESPACE) || isTerminator(TType::Comment))
        {
            gen_->next_();
        }
        return true;
    }

    bool JsonParser::isTerminator(JsonParser::TType type)
    {
        return gen_->current_().type_ == type;
    }

    /************************JsonParser_R***********************/

    R_JsonParser::R_JsonParser(std::shared_ptr<GenToken> gen) : gen_(gen),
                                                                expect_array_{}
    {
    }

    void R_JsonParser::set_builder(std::shared_ptr<ParserResultBuilder> b)
    {
        builder_ = b;
    }

    bool R_JsonParser::parser()
    {
        assert(builder_ && gen_);
        builder_->before_build();
        expect_array_.clear(); // 之后push_back或assign都一样
        gen_->next_();
        return json();
    }

    const std::vector<Type> &R_JsonParser::get_expect_token() const
    {
        return expect_array_;
    }

    bool R_JsonParser::json()
    {
        unuse();
        if (value())
        {
            unuse();
            if (isTerminator(TType::END))
            {
                builder_->after_build();
                return true;
            }
            expect_array_.push_back({TType::END});
            return false;
        }
        return false;
    }
    // 当前入口点
    enum W : char
    {
        Value0,
        Value1,
        While1,
        Value1_1,
        ObjEnd,
        Value2,
        While2,
        Value2_2,
        ArrEnd,
        AnyThing
    };

    /// 完全根据JsonParser的解析方法进行循环转换
    bool R_JsonParser::value()
    {
        std::stack<W> sw{};
        sw.push(AnyThing);
        sw.push(Value0);

        std::stack<std::size_t> nums{};

        std::stack<std::wstring> keys{};

        while (true)
        {
            auto w = sw.top();
            if (w == Value0)
            {
                sw.pop();
                do
                {
                    auto type = gen_->current_().type_;
                    if (type == Type::LBRACE)
                    {
                        gen_->next_();
                        unuse();
                        sw.push(ObjEnd);
                        if (isTerminator(TType::String))
                        {
                            keys.push(std::move(gen_->current_().value_));
                            gen_->next_();
                            unuse();
                            if (isTerminator(TType::COLON))
                            {
                                gen_->next_();
                                unuse();
                                sw.push(Value1);
                                continue;
                            }
                            else
                            {
                                expect_array_.push_back(TType::COLON);
                                return false;
                            }
                        }
                        else if (isTerminator(TType::RBRACE))
                        {
                            builder_->build_null_mbr();
                            sw.push(AnyThing);
                            break;
                        }
                        else
                        {
                            expect_array_.push_back(TType::String);
                            expect_array_.push_back(TType::RBRACE);
                            return false;
                        }
                    }
                    else if (type == Type::LSQUARE)
                    {
                        gen_->next_();
                        unuse();
                        sw.push(ArrEnd);
                        auto t = gen_->current_().type_;
                        if (t == TType::LBRACE || t == TType::LSQUARE || t == TType::String ||
                            t == TType::Number || t == TType::KeyWord || t == TType::Null)
                        {
                            sw.push(Value2);
                            continue;
                        }
                        else if (t == TType::RSQUARE)
                        {
                            builder_->build_null_mbr();
                            sw.push(AnyThing);
                            break;
                        }
                        else
                        {
                            expect_array_.assign({TType::RSQUARE});
                            return false;
                        }
                    }
                    else if (type == Type::String)
                    {
                        builder_->build_string(gen_->current_().value_);
                        gen_->next_();
                        sw.push(AnyThing);
                        break;
                    }
                    else if (type == Type::Number)
                    {
                        builder_->build_number(gen_->current_().value_);
                        gen_->next_();
                        sw.push(AnyThing);
                        break;
                    }
                    else if (type == Type::KeyWord)
                    {
                        builder_->build_keyword(gen_->current_().value_);
                        gen_->next_();
                        sw.push(AnyThing);
                        break;
                    }
                    else if (type == Type::Null)
                    {
                        builder_->build_Null(gen_->current_().value_);
                        gen_->next_();
                        sw.push(AnyThing);
                        break;
                    }
                    else
                    {
                        expect_array_.assign({TType::LBRACE, TType::LSQUARE, Type::String, Type::Number, Type::KeyWord, Type::Null});
                        return false;
                    }
                } while (true);
            }
            sw.pop();
            if (sw.empty())
                break; // return true
            w = sw.top();

            if (w == Value1)
            {
                unuse();
                builder_->set_memberkey(keys.top());
                keys.pop();
                builder_->can_start_iteration();
                if (isTerminator(TType::RBRACE))
                {
                    sw.pop();
                    sw.push(AnyThing);
                }
                else
                {
                    //                builder_->start_iteration();
                    nums.push(1);
                    sw.push(While1);
                    sw.push(AnyThing);
                }
            }
            else if (w == While1)
            {
                if (isTerminator(TType::COMMA))
                {
                    gen_->next_();
                    // if(member())
                    unuse();
                    if (isTerminator(TType::String))
                    {
                        keys.push(std::move(gen_->current_().value_));
                        gen_->next_();
                        unuse();
                        if (isTerminator(TType::COLON))
                        {
                            gen_->next_();
                            unuse();
                            sw.push(Value1_1);
                            sw.push(Value0);
                            continue;
                        }
                        else
                        {
                            expect_array_.push_back(TType::COLON);
                            return false;
                        }
                    }
                    else
                    {
                        expect_array_.push_back(TType::String);
                        return false;
                    }
                }
                else
                {
                    expect_array_.push_back(TType::RBRACE);
                    expect_array_.push_back(TType::COMMA);
                    // mb_ws=false;
                    return false;
                }
            }
            else if (w == Value1_1)
            {
                unuse();
                builder_->set_memberkey(keys.top());
                builder_->move_next();
                if (isTerminator(TType::RBRACE))
                {
                    builder_->finish_iteration();
                    keys.pop();
                    assert(nums.top() < std::numeric_limits<int>::max());
                    for (int i = 0; i < nums.top(); i++)
                    {
                        sw.pop();
                        sw.pop();
                    }
                    nums.pop();
                    // 等价于不写,arr同
                    // sw.pop();
                    // sw.push(AnyThing);
                }
                else
                {
                    nums.top()++;
                    keys.pop();
                    sw.push(While1);
                    sw.push(AnyThing);
                }
            }
            else if (w == ObjEnd)
            {
                if (isTerminator(TType::RBRACE))
                {
                    gen_->next_();
                    builder_->build_obj();
                    // 等价于不写,arr同
                    // sw.pop();
                    // sw.push(AnyThing);
                }
                else
                {
                    expect_array_.push_back(TType::RBRACE);
                    return false;
                }
            }
            else if (w == Value2)
            {
                unuse();
                builder_->can_start_iteration();
                if (isTerminator(Type::RSQUARE))
                {
                    sw.pop();
                    sw.push(AnyThing);
                }
                else
                {
                    //                builder_->start_iteration();
                    nums.push(1);
                    sw.push(While2);
                    sw.push(AnyThing);
                }
            }
            else if (w == While2)
            {
                if (isTerminator(Type::COMMA))
                {
                    gen_->next_();
                    unuse();
                    sw.push(Value2_2);
                    sw.push(Value0);
                    continue;
                }
                else
                {
                    expect_array_.push_back(Type::RSQUARE);
                    expect_array_.push_back(Type::COMMA);
                    return false;
                }
            }
            else if (w == Value2_2)
            {
                unuse();
                builder_->move_next();
                if (isTerminator(Type::RSQUARE))
                {
                    builder_->finish_iteration();
                    assert(nums.top() < std::numeric_limits<int>::max());
                    for (int i = 0; i < nums.top(); i++)
                    {
                        sw.pop();
                        sw.pop();
                    }
                    nums.pop();
                }
                else
                {
                    nums.top()++;
                    sw.push(While2);
                    sw.push(AnyThing);
                }
            }
            else if (w == ArrEnd)
            {
                if (isTerminator(TType::RSQUARE))
                {
                    gen_->next_();
                    builder_->build_arr();
                }
                else
                {
                    expect_array_.push_back(TType::RSQUARE);
                    return false;
                }
            }
            else
                ; // this will be an error and never happend
        }
        return true;
        // 以下注释是中间步骤，作为文档形式的一部分出现于此
        /*
            auto type=gen_->current_().type_;
            if(type==Type::LBRACE){
                //return obj();
                if(isTerminator(TType::LBRACE)){
                    gen_->next_();
                    //if(mb_ws())
                    bool mb_ws=false;
                    unuse();
                    //mb_ws_r()
                    //复用mb_ws是安全的,因为是赋值不是if判断
                    if(isTerminator(TType::String)){
                        auto key=std::move(gen_->current_().value_);
                        gen_->next_();
                        unuse();
                        if(isTerminator(TType::COLON)){
                            gen_->next_();
                            //if(element())
                            bool element=false;
                            unuse();
                            if(value())
                                element=unuse();
                            else
                                element=false;
                            //if(element()) end
                            if(element){
                                builder_->set_memberkey(key);
                                //return memberL()
                                //再次,复用mb_ws
                                if(isTerminator(TType::RBRACE)){
                                    mb_ws=true;
                                }else{
                                    builder_->start_iteration();
                                    auto rc=true;
                                    while (isTerminator(TType::COMMA)) {
                                            gen_->next_();
                                            //if(member())
                                            bool member=false;
                                            unuse();
                                            if(isTerminator(TType::String)){
                                                auto key=std::move(gen_->current_().value_);
                                                gen_->next_();
                                                unuse();
                                                if(isTerminator(TType::COLON)){
                                                    gen_->next_();
                                                    //if(element())
                                                    bool element=false;
                                                    unuse();
                                                    if(value())
                                                        element=unuse();
                                                    else
                                                        element=false;
                                                    //if(element()) end
                                                    if(element){
                                                        builder_->set_memberkey(key);
                                                        member=true;
                                                    }else
                                                        member=false;
                                                }else{
                                                    expect_array_.push_back(TType::COLON);
                                                    member=false;
                                                }
                                            }else{
                                                expect_array_.push_back(TType::String);
                                                member=false;
                                            }
                                            //if(member()) end
                                            if(member){
                                                builder_->move_next();
                                                if(isTerminator(TType::RBRACE)){
                                                    builder_->finish_iteration();
                                                    mb_ws=true;
                                                    rc=false;
                                                    break;
                                                }else
                                                    continue;
                                            }else{
                                                mb_ws=false;
                                                break;
                                            }
                                        }
                                    if(rc){
                                        expect_array_.push_back(TType::RBRACE);
                                        expect_array_.push_back(TType::COMMA);
                                        mb_ws=false;
                                    }
                                }
                                //return memberL() end
                            }else
                                mb_ws=false;
                        }else{
                            expect_array_.push_back(TType::COLON);
                            mb_ws=false;
                        }
                    }else if (isTerminator(TType::RBRACE)) {
                        builder_->build_null_mbr();
                        mb_ws=true;
                    }else{
                         expect_array_.push_back(TType::String);
                         expect_array_.push_back(TType::RBRACE);
                        mb_ws=false;
                    }
                    //mb_ws_r() end
                    ///mb_ws=mb_ws_r();
                    //if(mb_ws()) end
                    if(mb_ws){
                        if(isTerminator(TType::RBRACE)){
                            gen_->next_();
                            builder_->build_obj();
                            return true;
                        }else{
                            expect_array_.push_back(TType::RBRACE);
                            return false;
                        }
                    }else
                        return false;
                }else{
                    expect_array_.push_back(TType::LBRACE);
                    return false;
                }
                //obj() end
            }else if(type==Type::LSQUARE){
                if(isTerminator(TType::LSQUARE)){
                    gen_->next_();
                    //arr_ws()
                    bool arr_ws=false;
                    unuse();
                    //arr_ws_r()
                    //复用arr_ws
                    switch (gen_->current_().type_) {
                    case TType::LBRACE:
                    case TType::LSQUARE:
                    case TType::String:
                    case TType::Number:
                    case TType::KeyWord:
                    case TType::Null:{
                        if(value()){
                            unuse();
                            //elementsL()
                            //复用arr_ws
                            if(isTerminator(Type::RSQUARE))
                                arr_ws=true;
                            builder_->start_iteration();
                            if(isTerminator(Type::COMMA)){
                                while(isTerminator(Type::COMMA)){
                                    gen_->next_();
                                    //if(element())
                                    bool element=false;
                                    unuse();
                                    if(value())
                                        element=unuse();
                                    else
                                        element=false;
                                    //if(element()) end
                                    if(element){
                                        builder_->move_next();
                                        if(isTerminator(Type::RSQUARE)){
                                            builder_->finish_iteration();
                                            arr_ws=true;
                                            break;
                                        }else
                                            continue;
                                    }else{
                                        arr_ws=false;
                                        break;
                                    }
                                }
                            }else{
                                expect_array_.push_back(Type::RSQUARE);
                                expect_array_.push_back(Type::COMMA);
                                arr_ws=false;
                            }
                            //elementsL() end
                            ///arr_ws=elementsL();
                        }else
                            arr_ws=false;
                    }
                        break;
                    case TType::RSQUARE:
                        builder_->build_null_mbr();
                        arr_ws=true;
                        break;
                    default:
                        expect_array_.assign({TType::RSQUARE});
                        arr_ws=false;
                    }
                    //arr_ws_r() end
                    ///arr_ws=arr_ws_r();
                    //arr_ws() end
                    if(arr_ws){
                        if(isTerminator(TType::RSQUARE)){
                            gen_->next_();
                            builder_->build_arr();
                            return true;
                        }
                        expect_array_.push_back(TType::RSQUARE);
                        return false;
                    }
                    return false;
                }else{
                    expect_array_.push_back(TType::LSQUARE);
                    return false;
                }
            }else if(type==Type::String){
                builder_->build_string(gen_->current_().value_);
                gen_->next_();
                return true;
            }else if(type==Type::Number){
                builder_->build_number(gen_->current_().value_);
                gen_->next_();
                return true;
            }else if(type==Type::KeyWord){
                builder_->build_keyword(gen_->current_().value_);
                gen_->next_();
                return true;
            }else if(type==Type::Null){
                builder_->build_Null(gen_->current_().value_);
                gen_->next_();
                return true;
            }else{
                expect_array_.assign({Type::String,Type::Number,Type::KeyWord,Type::Null});
                return false;
            }
        */
    }

    bool R_JsonParser::unuse()
    {
        while (isTerminator(TType::WHITESPACE) || isTerminator(TType::Comment))
        {
            gen_->next_();
        }
        return true;
    }

    bool R_JsonParser::isTerminator(TType type)
    {
        return gen_->current_().type_ == type;
    }

}
