
#include "qjsv8.h"
#include "quickjs.h"

// std::map<JSValue, void*>* s_JSValueMap = nullptr;
// 
// extern "C" void JS_SetUserdata(JSValue obj, void* opaque)
// {
//     if (!s_JSValueMap)
//         s_JSValueMap = new std::map<JSValue, void*>();
//     s_JSValueMap->insert(std::make_pair(obj, opaque));
// }
// 
// extern "C" void* JS_GetUserdata(JSValueConst obj)
// {
//     if (!s_JSValueMap)
//         s_JSValueMap = new std::map<JSValue, void*>();
//     std::map<JSValue, void*>::const_iterator it = s_JSValueMap->find(obj);
//     if (it != s_JSValueMap->end())
//         return it->second;
//     return nullptr;
// }

namespace v8 {

class MacroAssembler { // Ϊ���ܷ���v8::PropertyCallbackInfo��˽�б��������ò�����MacroAssembler
public:
    static JSValue objectTemplateGetter(JSContext* ctx, JSValueConst thisVal, void* userdata);
    static JSValue objectTemplateSetter(JSContext* ctx, JSValueConst thisVal, JSValueConst val, void* userdata);

    void* stub;
};

JSValue MacroAssembler::objectTemplateGetter(JSContext* ctx, JSValueConst thisVal, void* userdata)
{
    miniv8::V8Context* context = (miniv8::V8Context*)JS_GetContextOpaque(ctx);
    miniv8::V8Template::Accessor* accessor = (miniv8::V8Template::Accessor*)userdata;

    miniv8::V8String* name = new miniv8::V8String(accessor->name);
    name->ensureQjsVal(context);
    v8::Local<v8::Name> propertyName = (v8::Utils::convert<miniv8::V8String, v8::String>(name));

    miniv8::V8Object* thisValue = miniv8::V8Object::create(context, thisVal);
    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    void* returnValue[4] = { 0 };
    returnValue[0] = isolate;

    miniv8::V8External external(accessor->data);

    void* args[v8::PropertyCallbackInfo<v8::Value>::kArgsLength] = { 0 };
    args[v8::PropertyCallbackInfo<v8::Value>::kShouldThrowOnErrorIndex] = nullptr;
    args[v8::PropertyCallbackInfo<v8::Value>::kHolderIndex] = thisValue; // kHolderIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kIsolateIndex] = isolate; // kIsolateIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kReturnValueDefaultValueIndex] = nullptr; // kReturnValueDefaultValueIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kReturnValueIndex] = &returnValue[2]; //kReturnValueIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kDataIndex] = &external; // kDataIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kThisIndex] = thisValue; // kThisIndex

    MacroAssembler infoStub; // ����ٵģ���ʵֻ��Ϊ�˵�ַռλ
    infoStub.stub = args;
    v8::PropertyCallbackInfo<v8::Value>* info = (v8::PropertyCallbackInfo<v8::Value>*)&infoStub;

    accessor->nameGetter(propertyName, *info);

    void** retVal = &args[v8::PropertyCallbackInfo<v8::Value>::kReturnValueIndex];
    if (nullptr == *retVal)
        return JS_UNDEFINED;
    miniv8::V8Value* ret = v8::Utils::openHandle<v8::Value, miniv8::V8Value>((v8::Value*)(retVal));
    return ret->v(ret); // TODO:Free
}

JSValue MacroAssembler::objectTemplateSetter(JSContext* ctx, JSValueConst thisVal, JSValueConst val, void* userdata)
{
    miniv8::V8Context* context = (miniv8::V8Context*)JS_GetContextOpaque(ctx);
    miniv8::V8Template::Accessor* accessor = (miniv8::V8Template::Accessor*)userdata;

    miniv8::V8String* name = new miniv8::V8String(accessor->name);
    name->ensureQjsVal(context);
    v8::Local<v8::Name> propertyName = (v8::Utils::convert<miniv8::V8String, v8::String>(name));

    miniv8::V8Object* thisValue = miniv8::V8Object::create(context, thisVal);

    miniv8::V8Value* value = new miniv8::V8Value(context, val);// TODO:
    v8::Local<v8::Value> valueV8 = (v8::Utils::convert<miniv8::V8Value, v8::Value>(value)); 

    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    void* returnValue[4] = { 0 };
    returnValue[0] = isolate;

    miniv8::V8External external(accessor->data);

    void* args[v8::PropertyCallbackInfo<v8::Value>::kArgsLength] = { 0 };
    args[v8::PropertyCallbackInfo<v8::Value>::kShouldThrowOnErrorIndex] = nullptr;
    args[v8::PropertyCallbackInfo<v8::Value>::kHolderIndex] = thisValue; // kHolderIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kIsolateIndex] = isolate; // kIsolateIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kReturnValueDefaultValueIndex] = nullptr; // kReturnValueDefaultValueIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kReturnValueIndex] = &returnValue[2]; //kReturnValueIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kDataIndex] = &external; // kDataIndex
    args[v8::PropertyCallbackInfo<v8::Value>::kThisIndex] = thisValue; // kThisIndex

    MacroAssembler infoStub; // ����ٵģ���ʵֻ��Ϊ�˵�ַռλ
    infoStub.stub = args;
    v8::PropertyCallbackInfo<void>* info = (v8::PropertyCallbackInfo<void>*) & infoStub;
    accessor->nameSetter(propertyName, valueV8, *info);

    void** retVal = &args[v8::PropertyCallbackInfo<v8::Value>::kReturnValueIndex];
    if (nullptr == *retVal)
        return JS_UNDEFINED;

    miniv8::V8Value* ret = v8::Utils::openHandle<v8::Value, miniv8::V8Value>((v8::Value*)(retVal));
    return ret->v(ret); // TODO:Free
}

} // v8

namespace miniv8 {

static inline uint16_t byteswap_ushort(uint16_t number)
{
#if defined(_MSC_VER) && _MSC_VER > 1310
    return _byteswap_ushort(number);
#elif defined(__GNUC__)
    return __builtin_bswap16(number);
#else
    return (number >> 8) | (number << 8);
#endif
}

////////////////////////////////////////
//     ����ת��������С�����½���     //
////////////////////////////////////////

// ��UTF16�����ַ�����������Ҫ��BOM���
// std::string utf16ToUtf8(const std::wstring& u16str)
// {
//     if (u16str.empty()) {
//         return std::string();
//     }
//     //Byte Order Mark
//     char16_t bom = u16str[0];
//     switch (bom) {
//     case 0xFEFF:	//Little Endian
//         return utf16leToUtf8(u16str);
//         break;
//     case 0xFFFE:	//Big Endian
//         return utf16beToUtf8(u16str);
//         break;
//     default:
//         return std::string();
//     }
// }
// 
// 
// // ��UTF16 LE������ַ�������
// std::string utf16leToUtf8(const std::wstring& u16str)
// {
//     if (u16str.empty()) {
//         return std::string();
//     }
//     const wchar_t* p = u16str.data();
//     std::wstring::size_type len = u16str.length();
//     if (p[0] == 0xFEFF) {
//         p += 1;	//����bom��ǣ�����
//         len -= 1;
//     }
// 
//     // ��ʼת��
//     std::string u8str;
//     u8str.reserve(len * 3);
// 
//     char16_t u16char;
//     for (std::wstring::size_type i = 0; i < len; ++i) {
//         // �����������С������(���������)
//         u16char = p[i];
// 
//         // 1�ֽڱ�ʾ����
//         if (u16char < 0x0080) {
//             // u16char <= 0x007f
//             // U- 0000 0000 ~ 0000 07ff : 0xxx xxxx
//             u8str += ((char)(u16char & 0x00FF));	// ȡ��8bit
//             continue;
//         }
//         // 2 �ֽ��ܱ�ʾ����
//         if (u16char >= 0x0080 && u16char <= 0x07FF) {
//             // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
//             u8str += ((char)(((u16char >> 6) & 0x1F) | 0xC0));
//             u8str += ((char)((u16char & 0x3F) | 0x80));
//             continue;
//         }
//         // ������Բ���(4�ֽڱ�ʾ)
//         if (u16char >= 0xD800 && u16char <= 0xDBFF) {
//             // * U-00010000 - U-001FFFFF: 1111 0xxx 10xxxxxx 10xxxxxx 10xxxxxx
//             uint32_t highSur = u16char;
//             uint32_t lowSur = p[++i];
//             // �Ӵ�����Ե�UNICODE�����ת��
//             // 1���Ӹߴ������ȥ0xD800����ȡ��Ч10bit
//             // 2���ӵʹ������ȥ0xDC00����ȡ��Ч10bit
//             // 3������0x10000����ȡUNICODE�����ֵ
//             uint32_t codePoint = highSur - 0xD800;
//             codePoint <<= 10;
//             codePoint |= lowSur - 0xDC00;
//             codePoint += 0x10000;
//             // תΪ4�ֽ�UTF8�����ʾ
//             u8str += ((char)((codePoint >> 18) | 0xF0));
//             u8str += ((char)(((codePoint >> 12) & 0x3F) | 0x80));
//             u8str += ((char)(((codePoint >> 06) & 0x3F) | 0x80));
//             u8str += ((char)((codePoint & 0x3F) | 0x80));
//             continue;
//         }
//         // 3 �ֽڱ�ʾ����
//         {
//             // * U-0000E000 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
//             u8str += ((char)(((u16char >> 12) & 0x0F) | 0xE0));
//             u8str += ((char)(((u16char >> 6) & 0x3F) | 0x80));
//             u8str += ((char)((u16char & 0x3F) | 0x80));
//             continue;
//         }
//     }
// 
//     return u8str;
// }
// 
// // ��UTF16BE�����ַ�������
// std::string utf16beToUtf8(const std::wstring& u16str)
// {
//     if (u16str.empty()) {
//         return std::string();
//     }
//     const wchar_t* p = u16str.data();
//     std::wstring::size_type len = u16str.length();
//     if (p[0] == 0xFEFF) {
//         p += 1;	//����bom��ǣ�����
//         len -= 1;
//     }
// 
//     // ��ʼת��
//     std::string u8str;
//     u8str.reserve(len * 2);
//     char16_t u16char;	//u16le ���ֽڴ��λ�����ֽڴ��λ
//     for (std::wstring::size_type i = 0; i < len; ++i) {
//         // �����������С������(���������)
//         u16char = p[i];
//         // �������תΪС����
//         u16char = byteswap_ushort(u16char);
// 
//         // 1�ֽڱ�ʾ����
//         if (u16char < 0x0080) {
//             // u16char <= 0x007f
//             // U- 0000 0000 ~ 0000 07ff : 0xxx xxxx
//             u8str += ((char)(u16char & 0x00FF));
//             continue;
//         }
//         // 2 �ֽ��ܱ�ʾ����
//         if (u16char >= 0x0080 && u16char <= 0x07FF) {
//             // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
//             u8str += ((char)(((u16char >> 6) & 0x1F) | 0xC0));
//             u8str += ((char)((u16char & 0x3F) | 0x80));
//             continue;
//         }
//         // ������Բ���(4�ֽڱ�ʾ)
//         if (u16char >= 0xD800 && u16char <= 0xDBFF) {
//             // * U-00010000 - U-001FFFFF: 1111 0xxx 10xxxxxx 10xxxxxx 10xxxxxx
//             uint32_t highSur = u16char;
//             uint32_t lowSur = byteswap_ushort(p[++i]);
//             // �Ӵ�����Ե�UNICODE�����ת��
//             // 1���Ӹߴ������ȥ0xD800����ȡ��Ч10bit
//             // 2���ӵʹ������ȥ0xDC00����ȡ��Ч10bit
//             // 3������0x10000����ȡUNICODE�����ֵ
//             uint32_t codePoint = highSur - 0xD800;
//             codePoint <<= 10;
//             codePoint |= lowSur - 0xDC00;
//             codePoint += 0x10000;
//             // תΪ4�ֽ�UTF8�����ʾ
//             u8str += ((char)((codePoint >> 18) | 0xF0));
//             u8str += ((char)(((codePoint >> 12) & 0x3F) | 0x80));
//             u8str += ((char)(((codePoint >> 06) & 0x3F) | 0x80));
//             u8str += ((char)((codePoint & 0x3F) | 0x80));
//             continue;
//         }
//         // 3 �ֽڱ�ʾ����
//         {
//             // * U-0000E000 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
//             u8str += ((char)(((u16char >> 12) & 0x0F) | 0xE0));
//             u8str += ((char)(((u16char >> 6) & 0x3F) | 0x80));
//             u8str += ((char)((u16char & 0x3F) | 0x80));
//             continue;
//         }
//     }
//     return u8str;
// }
// 
// // ��ȡת��ΪUTF-16 LE������ַ���
// std::wstring utf8ToUtf16le(const std::string& u8str, bool addbom, bool* ok)
// {
//     std::wstring u16str;
//     u16str.reserve(u8str.size());
//     if (addbom) {
//         u16str += (0xFEFF);	//bom (�ֽڱ�ʾΪ FF FE)
//     }
//     std::string::size_type len = u8str.length();
// 
//     const unsigned char* p = (unsigned char*)(u8str.data());
//     // �ж��Ƿ����BOM(�жϳ���С��3�ֽڵ����)
//     if (len > 3 && p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
//         p += 3;
//         len -= 3;
//     }
// 
//     bool is_ok = true;
//     // ��ʼת��
//     for (std::string::size_type i = 0; i < len; ++i) {
//         uint32_t ch = p[i];	// ȡ��UTF8�������ֽ�
//         if ((ch & 0x80) == 0) {
//             // ���λΪ0��ֻ��1�ֽڱ�ʾUNICODE�����
//             u16str += ((char16_t)ch);
//             continue;
//         }
//         switch (ch & 0xF0)
//         {
//         case 0xF0: // 4 �ֽ��ַ�, 0x10000 �� 0x10FFFF
//         {
//             uint32_t c2 = p[++i];
//             uint32_t c3 = p[++i];
//             uint32_t c4 = p[++i];
//             // ����UNICODE�����ֵ(��һ���ֽ�ȡ��3bit������ȡ6bit)
//             uint32_t codePoint = ((ch & 0x07U) << 18) | ((c2 & 0x3FU) << 12) | ((c3 & 0x3FU) << 6) | (c4 & 0x3FU);
//             if (codePoint >= 0x10000)
//             {
//                 // ��UTF-16�� U+10000 �� U+10FFFF ������16bit��Ԫ��ʾ, �������.
//                 // 1����������ȥ0x10000(�õ�����Ϊ20bit��ֵ)
//                 // 2��high ������ �ǽ���20bit�еĸ�10bit����0xD800(110110 00 00000000)
//                 // 3��low  ������ �ǽ���20bit�еĵ�10bit����0xDC00(110111 00 00000000)
//                 codePoint -= 0x10000;
//                 u16str += ((char16_t)((codePoint >> 10) | 0xD800U));
//                 u16str += ((char16_t)((codePoint & 0x03FFU) | 0xDC00U));
//             } else
//             {
//                 // ��UTF-16�� U+0000 �� U+D7FF �Լ� U+E000 �� U+FFFF ��Unicode�����ֵ��ͬ.
//                 // U+D800 �� U+DFFF ����Ч�ַ�, Ϊ�˼���������������������(������򲻱���)
//                 u16str += ((char16_t)codePoint);
//             }
//         }
//         break;
//         case 0xE0: // 3 �ֽ��ַ�, 0x800 �� 0xFFFF
//         {
//             uint32_t c2 = p[++i];
//             uint32_t c3 = p[++i];
//             // ����UNICODE�����ֵ(��һ���ֽ�ȡ��4bit������ȡ6bit)
//             uint32_t codePoint = ((ch & 0x0FU) << 12) | ((c2 & 0x3FU) << 6) | (c3 & 0x3FU);
//             u16str += ((char16_t)codePoint);
//         }
//         break;
//         case 0xD0: // 2 �ֽ��ַ�, 0x80 �� 0x7FF
//         case 0xC0:
//         {
//             uint32_t c2 = p[++i];
//             // ����UNICODE�����ֵ(��һ���ֽ�ȡ��5bit������ȡ6bit)
//             uint32_t codePoint = ((ch & 0x1FU) << 12) | ((c2 & 0x3FU) << 6);
//             u16str += ((char16_t)codePoint);
//         }
//         break;
//         default:	// ���ֽڲ���(ǰ���Ѿ����������Բ�Ӧ�ý���)
//             is_ok = false;
//             break;
//         }
//     }
//     if (ok != NULL) {
//         *ok = is_ok;
//     }
// 
//     return u16str;
// }
// 
// // ��ȡת��ΪUTF-16 BE���ַ���
// std::wstring utf8ToUtf16be(const std::string& u8str, bool addbom, bool* ok)
// {
//     // �Ȼ�ȡutf16le�����ַ���
//     std::wstring u16str = utf8ToUtf16le(u8str, addbom, ok);
//     // ��С����ת��Ϊ�����
//     for (size_t i = 0; i < u16str.size(); ++i) {
//         u16str[i] = byteswap_ushort(u16str[i]);
//     }
//     return u16str;
// }

//�������ֽ�,��ȡutf8�ַ���ռ�ֽ���
inline int getUtf8charByteNum(unsigned char ch)
{
    int byteNum = 0;

    if (ch >= 0xFC && ch < 0xFE)
        byteNum = 6;
    else if (ch >= 0xF8)
        byteNum = 5;
    else if (ch >= 0xF0)
        byteNum = 4;
    else if (ch >= 0xE0)
        byteNum = 3;
    else if (ch >= 0xC0)
        byteNum = 2;
    else if (0 == (ch & 0x80))
        byteNum = 1;

    return byteNum;
}
#include <string.h>
//����utf8�ַ����ַ�����
size_t getUtf8Length(const char* str, size_t clen)
{
    size_t len = 0;
    int byteNum = 0;
    unsigned char ch;
    const char* ptr = str;

    if (NULL == str)
        return 0;

    if (0 == clen)
        clen = strlen(str);

    while (*ptr != '\0' && len < clen) {
        ch = (unsigned char)*ptr;
        if (0 == (byteNum = getUtf8charByteNum(ch)))
            return 0;
        ptr += byteNum;
        len++;
    }

    return len;
}

uint32_t hash(const char* arKey, size_t nKeyLength)
{
    uint32_t h = 0, g;
    const char* arEnd = arKey + nKeyLength;

    while (arKey < arEnd) {
        h = (h << 4) + *arKey++;
        if ((g = (h & 0xF0000000))) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return h;
}

V8Isolate* V8Isolate::m_inst = nullptr;

JSClassID g_mbClassId = 0;

V8Isolate::V8Isolate()
{
    m_runtime = JS_NewRuntime();
    if (m_inst)
        DebugBreak();

    JS_NewClassID(&g_mbClassId);

    m_globalizeHandles.resize(kHandlesSize);
    m_handleScopeHandles.resize(kHandlesSize);
    memset(m_globalizeHandles.data(), 0, sizeof(void*) * m_globalizeHandles.size());
    memset(m_handleScopeHandles.data(), 0, sizeof(void*) * m_handleScopeHandles.size());

    typedef v8::internal::Internals I;
    *I::GetRoot((v8::Isolate *)&m_apiPointer, I::kNullValueRootIndex) = (v8::internal::Object *)(new miniv8::V8Value(nullptr, JS_NULL));
    *I::GetRoot((v8::Isolate *)&m_apiPointer, I::kTrueValueRootIndex) = (v8::internal::Object *)(new miniv8::V8Value(nullptr, JS_TRUE));
    *I::GetRoot((v8::Isolate *)&m_apiPointer, I::kFalseValueRootIndex) = (v8::internal::Object *)(new miniv8::V8Value(nullptr, JS_FALSE));
    *I::GetRoot((v8::Isolate *)&m_apiPointer, I::kUndefinedValueRootIndex) = (v8::internal::Object *)(new miniv8::V8Value(nullptr, JS_UNDEFINED));
    *I::GetRoot((v8::Isolate *)&m_apiPointer, I::kEmptyStringRootIndex) = (v8::internal::Object *)(new miniv8::V8String(""));

    m_inst = this;
}

void** V8Isolate::findGlobalizeHandleEmptyIndex()
{
    for (size_t i = 0; i < m_globalizeHandles.size(); ++i) {
        if (nullptr == m_globalizeHandles[i]) {
            return (void**)(m_globalizeHandles.data() + i);
        }
    }
    DebugBreak();
    return nullptr;
}

void** V8Isolate::findHandleScopeEmptyIndex()
{
    for (size_t i = 0; i < m_handleScopeHandles.size(); ++i) {
        if (nullptr == m_handleScopeHandles[i]) {
            return (void**)(m_handleScopeHandles.data() + i);
        }
    }
    DebugBreak();
    return nullptr;
}

void V8Isolate::enterContext(V8Context* ctx)
{
    m_currentCtxs.push_back(ctx);
}

void V8Isolate::exitContext()
{
    m_currentCtxs.pop_back();
}

V8Context* V8Isolate::getCurrentCtx()
{
    if (isInContext())
        return m_currentCtxs[m_currentCtxs.size() - 1];

    return nullptr;
}

// V8Context* V8Isolate::setCurrentCtx(V8Context* ctx)
// {
//     V8Context* oldCtx = m_currentCtx;
//     m_currentCtx = ctx;
//     return oldCtx;
// }

V8Isolate::~V8Isolate() {

}

bool V8Isolate::isInContext()
{
    return m_currentCtxs.size() > 0;
}

V8Value* V8Value::create(V8Context* ctx, JSValue value)
{
    if (JS_IsString(value))
        return new V8String(ctx, value);
    return new V8Value(ctx, value);
}

JSClassID V8Object::s_classId = 0;

V8Object* V8Object::create(V8Context* ctx, JSValue value)
{
    s_classId = g_mbClassId; // TODO: �Ժ�������һ��

    V8Object* old = (V8Object*)JS_GetUserdata(value);
    if (old) {
        JSValue oldV = JS_GetTestVal(value);
        miniv8ReleaseAssert(JS_VALUE_IS_EQ(oldV,value), "V8Object::create fail\n");
        return old;
    }
    // TODO:JS_IsUndefined\JS_IsNull
    return new V8Object(ctx, value);
}

V8Object::V8Object(V8Context* ctx, JSValue value) : V8Value(ctx, value)
{
    m_type = kObjectTypeObject;
    m_props = nullptr;
    isObject = true;
    m_internalFieldCount = 0;

    static int s_count = 0;
    s_count++;
    if (s_count == 27)
        printDebug("V8Object, %d, %I64u!!\n", s_count, value);

    if (s_count == 8)
        printDebug("V8Object, %d, %I64u!!\n", s_count, value);

    printDebug("V8Object: %p, %d, %I64u\n", this, s_count, value);

    V8Object* old = (V8Object*)JS_GetUserdata(value/*, JS_CLASS_OBJECT*/);
    if (old)
        DebugBreak();

    JS_SetUserdata(value, this);
    JS_SetTestVal(value, value);
}

V8Object::~V8Object()
{
    // TODO: �ͷŵ�ʱ��ҪJS_GetUserdata
    if (m_props)
        delete m_props;
}

JSValue V8Function::onV8FunctionWrap(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv, void* userdata)
{
    miniv8::V8Function* self = (miniv8::V8Function*)userdata;
    miniv8::V8Isolate* isolate = miniv8::V8Isolate::GetCurrent();

    miniv8::V8Context* context = (miniv8::V8Context*)JS_GetContextOpaque(ctx);
    miniv8ReleaseAssert(!!context, "onV8FunctionWrap fail\n");
    miniv8::V8Object* thisValV8 = miniv8::V8Object::create(context, self->v(self));

    miniv8::V8Data dataV8(context, self->v(self));

    std::vector<void*> argValue;
    argValue.resize(argc + 2);

    for (int i = 0; i < argc; ++i) {
        miniv8::V8Value* v = new miniv8::V8Value(context, argv[argc - i - 1]);
        argValue[i] = v;
    }
    argValue[argc] = miniv8::V8Object::create(context, thisVal);
    argValue[argc + 1] = argValue[argc]; // ���argc��0�Ļ���v8����FunctionCallbackInfo<T>::This()��ȡ�������ַ

    void* returnValue[4] = { 0 };
    returnValue[1] = isolate;

    V8External external(self->m_data);

    void* implicitArgs[v8::FunctionCallbackInfo<v8::Value>::kArgsLength] = { 0 };
    implicitArgs[0] = argValue[argc]; // kHolderIndex
    implicitArgs[1] = isolate; // kIsolateIndex
    implicitArgs[2] = nullptr; // kReturnValueDefaultValueIndex
    implicitArgs[3] = &returnValue; //kReturnValueIndex
    implicitArgs[4] = &external; // kDataIndex
    implicitArgs[5] = nullptr; // kCalleeIndex
    implicitArgs[6] = thisValV8; // kContextSaveIndex
    implicitArgs[7] = nullptr; // kNewTargetIndex
    
    FunctionCallbackInfoWrap functionCallbackInfoWrap;
    functionCallbackInfoWrap.implicitArgs = implicitArgs;
    functionCallbackInfoWrap.values = &argValue[argc/* -1 */];
    functionCallbackInfoWrap.length = argc;

    v8::FunctionCallbackInfo<v8::Value>* functionCallbackInfoWrapV8 = (v8::FunctionCallbackInfo<v8::Value>*)&functionCallbackInfoWrap;

    self->m_callback(*functionCallbackInfoWrapV8);

    v8::ReturnValue<v8::Value>* returnValueV8 = (v8::ReturnValue<v8::Value>*)(&returnValue[3]);
    if (nullptr == *(void**)returnValueV8)
        return JS_UNDEFINED;
    
    v8::Local<v8::Value> retV8 = returnValueV8->Get();
    miniv8::V8Value* ret = (miniv8::V8Value*)(*retV8);

    for (int i = 0; i < argc; ++i) {
        miniv8::V8Value* v = (miniv8::V8Value*)returnValue[i];
        delete v;
    }

    return ret->v(ret); // TODO:Free
}

std::string V8Function::getName()
{
    if (m_name.empty()) {
        static int i = 0;
        ++i;
        char temp[64] = { 0 };
        sprintf(temp, "_miniv8_func_noname_%d\n", i);
        m_name = temp;
    }
    return m_name;
}

void V8Function::setName(const std::string& name)
{
    if (!m_name.empty())
        DebugBreak();
    m_name = name;
}

JSValue V8Function::V8Function_v(V8Data* ptr)
{
    V8Function* self = (V8Function*)ptr;
    if (!JS_IsNull(self->m_qjsValue))
        return self->m_qjsValue;
        
    std::string name = self->getName();
    self->m_qjsValue = JS_NewCFunction2(self->m_ctx->ctx(), onV8FunctionWrap, name.c_str(), name.size(), JS_CFUNC_constructor, 0, self);
    return self->m_qjsValue;
}

void V8FunctionTemplate::setClassName(const std::string& name)
{
    if (!m_name.empty() && m_name != name)
        DebugBreak();
    m_name = name;
}

std::string V8FunctionTemplate::getClassName()
{
    if (m_name.empty()) {
        static int i = 0;
        ++i;
        char temp[64] = { 0 };
        sprintf(temp, "_miniv8_funtemp_noname_%d\n", i);
        m_name = temp;
    }
    return m_name;
}

void V8FunctionTemplate::functionTemplateCall(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    // �ڹ���ʵ����ʱ�򣬰�InstanceTemplate
    miniv8::V8FunctionTemplate* self = (miniv8::V8FunctionTemplate*)(v8::External::Cast(*info.Data())->Value());
    V8ObjectTemplate* objTempl = (V8ObjectTemplate*)self->m_instanceTemplate;
    if (!objTempl)
        return;

    V8Isolate* isolate = (V8Isolate*)info.GetIsolate();
    V8Context* ctx = isolate->getCurrentCtx();

    v8::Local<v8::Object> objV8 = info.This();
    miniv8::V8Object* obj = v8::Utils::openHandle<v8::Object, miniv8::V8Object>(*objV8);

    std::map<std::string, JSValue>::iterator it = objTempl->m_props.begin();
    for (; it != objTempl->m_props.end(); ++it) {
        std::string name = it->first;
        JSValue val = it->second;
        JSAtom prop = JS_NewAtomLen(ctx->ctx(), name.c_str(), name.size());

        JS_DefinePropertyValue(ctx->ctx(), obj->v(obj), prop, val, 0);
    }

    // TODO: ��setter

//     JSCFunctionListEntry jsMt19937ProtoFuncs[2] = {
//         //JS_CFUNC_DEF("generate", 1, jsMt19937Generate)
//         { "generate", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0,{ 1, JS_CFUNC_generic,{ jsMt19937Generate } } },
//         { "haha", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CGETSET, 0,{ 0 } }
//     };
// 
//     JS_SetPropertyFunctionList(ctx->ctx(), obj->v(), jsMt19937ProtoFuncs, /*std::extent_v<decltype(jsMt19937ProtoFuncs)>*/ 2);
}

// typedef void (*AccessorNameGetterCallback)(
//     Local<Name> property,
//     const PropertyCallbackInfo<Value>& info);
// 
// static const int kShouldThrowOnErrorIndex = 0;
// static const int kHolderIndex = 1;
// static const int kIsolateIndex = 2;
// static const int kReturnValueDefaultValueIndex = 3;
// static const int kReturnValueIndex = 4;
// static const int kDataIndex = 5;
// static const int kThisIndex = 6;

void V8Template::newTemplateInstance(miniv8::V8Context* ctx, miniv8::V8Object* obj)
{
    JSValue v = obj->v(obj);
    obj->m_internalFieldCount = this->m_internalFieldCount > obj->m_internalFieldCount ? this->m_internalFieldCount : obj->m_internalFieldCount;

    printDebug("v8::ObjectTemplate::NewInstance: %p, obj->m_internalFieldCount:%d\n", obj, obj->m_internalFieldCount);

    // ע�⣬���ʵ�����Ǹ�function template��ԭ��������Ҫ���⴦��һ��
    for (std::map<std::string, JSValue>::iterator it = this->m_props.begin(); it != this->m_props.end(); ++it) {
        std::string name = it->first;
        JSValue val = it->second;
        JSAtom prop = JS_NewAtomLen(ctx->ctx(), name.c_str(), name.size());

        JS_DefinePropertyValue(ctx->ctx(), v, prop, val, 0);
    }

    if (this->m_accessors.empty())
        return;

    JSCFunctionListEntry* accessorsFuncs = new JSCFunctionListEntry[this->m_accessors.size()];
    std::map<std::string, V8Template::Accessor>::iterator it = this->m_accessors.begin();
    for (int i = 0; it != this->m_accessors.end(); ++it, ++i) {
        const std::string& name = it->first;
        V8Template::Accessor* val = &it->second;
        miniv8ReleaseAssert(val->nameGetter || val->nameSetter, "V8Template::newTemplateInstance fail\n");

        accessorsFuncs[i].name = name.c_str();
        accessorsFuncs[i].prop_flags = JS_PROP_CONFIGURABLE;
        accessorsFuncs[i].def_type = JS_DEF_CGETSET;
        accessorsFuncs[i].magic = 0;
        accessorsFuncs[i].u.getset.get.getter = nullptr;
        accessorsFuncs[i].u.getset.set.setter = nullptr;

        if (val->nameGetter)
            accessorsFuncs[i].u.getset.get.getter = &v8::MacroAssembler::objectTemplateGetter;
        if (val->nameSetter)
            accessorsFuncs[i].u.getset.set.setter = &v8::MacroAssembler::objectTemplateSetter;
        accessorsFuncs[i].u.getset.userdata = val;
    }
    JS_SetPropertyFunctionList(ctx->ctx(), v, accessorsFuncs, this->m_accessors.size());
}

void V8String::ensureQjsVal(V8Context* ctx)
{
    V8Context* m_ctx = ctx;
    m_qjsValue = JS_NewStringLen(ctx->ctx(), m_str.c_str(), m_str.size());
    m_str = "";
}

JSValue V8String::V8String_v(V8Data* ptr)
{
    V8String* self = (V8String*)ptr;
    miniv8::V8Isolate* isolate = miniv8::V8Isolate::GetCurrent();
    V8Context* ctx = isolate->getCurrentCtx();
    self->ensureQjsVal(ctx);

    return self->m_qjsValue;
}

std::string V8String::getStr()
{
    if (!m_str.empty())
        return m_str;

    size_t len = 0;

    JSContext* context = ctx();
    if (!context) {
        miniv8::V8Isolate* isolate = miniv8::V8Isolate::GetCurrent();
        V8Context* currentCtx = isolate->getCurrentCtx();
        if (currentCtx)
            context = currentCtx->ctx();
    }
    const char* nameString = JS_ToCStringLen(context, &len, m_qjsValue);
    std::string ret = nameString;
    JS_FreeCString(ctx(), nameString);
    return ret;
}

}