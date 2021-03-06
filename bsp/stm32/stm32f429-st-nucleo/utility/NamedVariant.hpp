#pragma once

#include <vector>
using std::vector;
//#if 0
//#include <string>
//using std::string;
//using std::to_string;
//#else
#include <cmath>
#include "cstring.hpp"
using string = ASIO::cstring;
using ASIO::npos;
using ASIO::to_string;
//#endif // 0

#ifndef NAMEDVARIANT_MAXNAMELEN
#define NAMEDVARIANT_MAXNAMELEN 20
#endif

#ifndef NAMEDVARIANT_SUPPORT_DOUBLE
#define NAMEDVARIANT_SUPPORT_DOUBLE 0
#endif
const char doublesupport_hint[] = \
    "variant:please enable double support before use it\n";

namespace variant
{
    union Variant
    {
        int i32;
        float f32;
#if NAMEDVARIANT_SUPPORT_DOUBLE
        double d64;
#endif
    };
    int typename_to_index(const char*);
    int string_to_variant(const char*, int, Variant*);
    
    class NamedVariant
    {
    public:
        enum VariantType
        {
            //    type_char = 0,
            //    type_uint8_t = type_char,
            
            type_int,
            type_float,
            type_double,
            type_unknown = 0xFF
        };
        NamedVariant() {
            memset(this, 0, sizeof(*this));
        }
        ~NamedVariant() {}
        
        //TODO:无论输入是否有';'都应该正常运行
        NamedVariant from_line(const string& src,bool* ok=nullptr)
        {
            bool retval = false;
            vector<string> vstr = src.split_cascade(",=;");
            name[0] = '\0';
            typeinfo = type_unknown;
            v.i32 = 0;
            if (vstr.size() < 3)
            {
                goto exit;
            }
//            const string &_name = vstr[0], &_type = vstr[1], &_value = vstr[2];
            if (vstr[0].empty() || vstr[1].empty() || vstr[2].empty())
            {
                goto exit;
            }
            vstr[0].copyTo(name);
            typeinfo = typename_to_index(vstr[1].c_str());
            string_to_variant(vstr[2].c_str(), typeinfo, &v);
            retval = true;
exit:
            if (ok != nullptr)
                *ok = retval;
            return *this;
        }

        //print variant in format:"VAR_NAME,VAR_TYPE=VAR_VALUE"
        string to_line()
        {
            string result = name;
            switch (typeinfo)
            {
            case type_int:result += ",int=" + ASIO::to_string(v.i32); break;
            case type_float:result += ",float=" + ASIO::to_string(v.f32); break;
            #if NAMEDVARIANT_SUPPORT_DOUBLE
            case type_double:result += ",double=" + std::to_string(v.d64); break;
            #else
            case type_double:result += ",double=NaN";
                printf("%s", doublesupport_hint);
                break;
                #endif
            default:
                result += ",unknown=0"; break;
            }
            result += ";";
            return result;
        }

        char name[NAMEDVARIANT_MAXNAMELEN];
        int32_t typeinfo; //only lowest byte used,high byte reserved
        Variant v;
    private:
    
    };

    int typename_to_index(const char* s)
    {
        if (strcmp(s, "int") == 0)
            return NamedVariant::type_int;
        else if (strcmp(s, "float") == 0)
            return NamedVariant::type_float;
        else if (strcmp(s, "double") == 0)
            return NamedVariant::type_double;
        else
            return NamedVariant::type_unknown;
    }
        
    int string_to_variant(const char* s, int type, Variant* v)
    {
        switch (type)
        {
        case NamedVariant::type_int:v->i32 = atoi(s); break;
        case NamedVariant::type_float:v->f32 = atof(s); break;//atof只支持double，strtof支持float，但是需要stdlib
#if NAMEDVARIANT_SUPPORT_DOUBLE
        case NamedVariant::type_double:v->d64 = atof(s); break;
#else
        case NamedVariant::type_double:v->f32 = NAN;
            printf("%s", doublesupport_hint);
            return -1;
            break;
#endif
        default:
            v->i32 = 0;
            return -1;
            break;
        }
        return 0;
    }

    vector<NamedVariant> string_split_to_variant(const string& s, bool *ok = nullptr)
    {
        vector<NamedVariant> vars;
        NamedVariant var;
        bool retval = true;
        string spliter = ";";
        vector<string> lines;
        if (s.find(";\r\n") != npos)
            spliter = ";\r\n";
        else if (s.find(";\n") != npos)
            spliter = ";\n";
        else if (s.find(";") != npos)
            spliter = ";";
        else {
            printf("no variant spliter find\n");
            goto exit;
        }

        lines = s.split(spliter.c_str());
        for (auto l : lines)
        {
            var.from_line(l, &retval);
            if (!retval)
                goto exit;
            vars.push_back(var);
        }
        
exit:
        if (ok != nullptr)
            *ok = retval;
        return vars;
    }
    
}
