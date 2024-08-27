#ifndef QJSV8_H_
#define QJSV8_H_
#include "../v8_5_7/include/v8-debug.h"
#include "../v8_5_7/include/v8-profiler.h"
#include "../v8_5_7/include/v8.h"

// #include "G:/mycode/mb_temp/v8_5_7/include/sys-info.h"

#include "quickjs.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

void printEmptyFuncInfo(const char *fun, bool isBreak);
void miniv8ReleaseAssert(bool b, const char *info);
void printDebug(const char *format, ...);

namespace v8 {

class Utils {
public:
  template <class From, class To>
  static inline To *openHandle(const From *that, bool allowEmptyHandle = false);

  template <class From, class To> static inline Local<To> convert(From *obj);

  // 使用宏定义生成具体的类型转换函数
#define DECL_ALLOW_TYPE_AS(className)                                          \
  static inline qjsv8::V8##className *fromV8(const v8::className *that);       \
  static inline qjsv8::V8##className *fromV8Local(                             \
      v8::Local<v8::className> *that);                                         \
  static inline qjsv8::V8##className *fromV8##className(void *that);           \
  static inline v8::Local<v8::className> toV8Local(                            \
      qjsv8::V8##className *that);                                             \
  static inline v8::Local<v8::className> toV8##className(qjsv8::V8Data *that); \
  static inline v8::Local<v8::className> toV8##className(void *that);
};

} // namespace v8

// 示例：生成特定的类型转换函数

namespace qjsv8 {

extern JSClassID g_mbClassId;

uint32_t hash(const char *arKey, size_t nKeyLength);
size_t getUtf8Length(const char *str, size_t clen);
std::string utf16ToUtf8(const std::wstring &u16str);
std::string utf16leToUtf8(const std::wstring &u16str);
std::string utf16beToUtf8(const std::wstring &u16str);
std::wstring utf8ToUtf16le(const std::string &u8str, bool addbom, bool *ok);
std::wstring utf8ToUtf16be(const std::string &u8str, bool addbom, bool *ok);

class V8Context;
class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
  ArrayBufferAllocator() {}
  void *Allocate(size_t length) override;
  void *AllocateUninitialized(size_t length) override;
  void Free(void *data, size_t length) override;
  static v8::ArrayBuffer::Allocator *NewDefaultAllocator() {
    static ArrayBufferAllocator defaultAllocator; // 静态局部变量，只初始化一次
    return &defaultAllocator;
  }
};
struct FunctionCallbackInfoWrap {
  void *implicitArgs;
  void *values;
  int length;
};

enum V8ObjectType : uint32_t {
  kObjectTypeContext,
  kObjectTypeData,
  kObjectTypePrivate,
  kObjectTypeValue,
  kObjectTypePrimitive,
  kObjectTypeName,
  kObjectTypeSymbol,
  kObjectTypeString,
  kObjectTypeBoolean,
  kObjectTypeAccessorSignature,
  kObjectTypeArray,
  kObjectTypeObject,
  kObjectTypeFunction,
  kObjectTypeExternal,
  kObjectTypeArrayBuffer,
  kObjectTypeInt16Array,
  kObjectTypeInt32Array,
  kObjectTypeInt8Array,
  kObjectTypeInt32,
  kObjectTypeInteger,
  kObjectTypeTemplate,
  kObjectTypeObjectTemplate,
  kObjectTypeFunctionTemplate,
};

class V8Isolate {
public:
  V8Isolate();
  ~V8Isolate();

  static V8Isolate *GetCurrent() {
    // JSContext *jsctx = JS_NewContext(m_inst->getRuntime());

    return m_inst; // TODO: save at tls
  }

  bool isInContext(void);
  void enterContext(V8Context *ctx);
  void exitContext();

  V8Context *getCurrentCtx();

  JSContext * getCurrentJsContext();
  // V8Context* setCurrentCtx(V8Context* ctx);

  JSRuntime *getRuntime() const { return m_runtime; }

  void **findGlobalizeHandleEmptyIndex();
  void **findHandleScopeEmptyIndex();

private:
  void *m_apiPointer[64]; // ����������ǰ�棬v8�Ļ������
  const int kHandlesSize = 0x10000;
  std::vector<void *> m_globalizeHandles;
  std::vector<void *> m_handleScopeHandles;
  JSRuntime *m_runtime;
  // V8Context* m_currentCtx;
  std::vector<V8Context *> m_currentCtxs;
  static V8Isolate *m_inst;
};

class V8StackTrace {
public:
  V8StackTrace() {}
  int getFrameCount() const { return 0; }
};

class V8Message;
class V8Value;
/**
 * An external exception handler.
 */
class V8TryCatch {
public:
  /**
   * Creates a new try/catch block and registers it with v8.  Note that
   * all TryCatch blocks should be stack allocated because the memory
   * location itself is compared against JavaScript try/catch blocks.
   */
  V8_DEPRECATED("Use isolate version", V8TryCatch());

  /**
   * Creates a new try/catch block and registers it with v8.  Note that
   * all TryCatch blocks should be stack allocated because the memory
   * location itself is compared against JavaScript try/catch blocks.
   */
  V8TryCatch(V8Isolate *isolate);

  /**
   * Unregisters and deletes this try/catch block.
   */
  ~V8TryCatch();

  /**
   * Returns true if an exception has been caught by this try/catch block.
   */
  bool HasCaught() const;

  /**
   * For certain types of exceptions, it makes no sense to continue execution.
   *
   * If CanContinue returns false, the correct action is to perform any C++
   * cleanup needed and then return.  If CanContinue returns false and
   * HasTerminated returns true, it is possible to call
   * CancelTerminateExecution in order to continue calling into the engine.
   */
  bool CanContinue() const;

  /**
   * Returns true if an exception has been caught due to script execution
   * being terminated.
   *
   * There is no JavaScript representation of an execution termination
   * exception.  Such exceptions are thrown when the TerminateExecution
   * methods are called to terminate a long-running script.
   *
   * If such an exception has been thrown, HasTerminated will return true,
   * indicating that it is possible to call CancelTerminateExecution in order
   * to continue calling into the engine.
   */
  bool HasTerminated() const;

  /**
   * Throws the exception caught by this TryCatch in a way that avoids
   * it being caught again by this same TryCatch.  As with ThrowException
   * it is illegal to execute any JavaScript operations after calling
   * ReThrow; the caller must return immediately to where the exception
   * is caught.
   */
  v8::Local<V8Value> ReThrow();

  /**
   * Returns the exception caught by this try/catch block.  If no exception has
   * been caught an empty handle is returned.
   *
   * The returned handle is valid until this TryCatch block has been destroyed.
   */
  v8::Local<V8Value> Exception() const;

  void setException(qjsv8::V8Value *ex, V8Message *message_obj);

  void setNext(V8TryCatch *next);

  /**
   * Returns the .stack property of the thrown object.  If no .stack
   * property is present an empty handle is returned.
   */
  V8_WARN_UNUSED_RESULT V8Value *StackTrace(V8Context *context) const;

  /**
   * Returns the message associated with this exception.  If there is
   * no message associated an empty handle is returned.
   *
   * The returned handle is valid until this TryCatch block has been
   * destroyed.
   */
  V8Message *Message() const;

  /**
   * Clears any exceptions that may have been caught by this try/catch block.
   * After this method has been called, HasCaught() will return false. Cancels
   * the scheduled exception if it is caught and ReThrow() is not called before.
   *
   * It is not necessary to clear a try/catch block before using it again; if
   * another exception is thrown the previously caught exception will just be
   * overwritten.  However, it is often a good idea since it makes it easier
   * to determine which operation threw a given exception.
   */
  void Reset();

  /**
   * Set verbosity of the external exception handler.
   *
   * By default, exceptions that are caught by an external exception
   * handler are not reported.  Call SetVerbose with true on an
   * external exception handler to have exceptions caught by the
   * handler reported as if they were not caught.
   */
  void SetVerbose(bool value);

  /**
   * Set whether or not this TryCatch should capture a Message object
   * which holds source information about where the exception
   * occurred.  True by default.
   */
  void SetCaptureMessage(bool value);

  /**
   * There are cases when the raw address of C++ TryCatch object cannot be
   * used for comparisons with addresses into the JS stack. The cases are:
   * 1) ARM, ARM64 and MIPS simulators which have separate JS stack.
   * 2) Address sanitizer allocates local C++ object in the heap when
   *    UseAfterReturn mode is enabled.
   * This method returns address that can be used for comparisons with
   * addresses into the JS stack. When neither simulator nor ASAN's
   * UseAfterReturn is enabled, then the address returned will be the address
   * of the C++ try catch handler itself.
   */
  static void *JSStackComparableAddress(V8TryCatch *handler) {
    if (handler == NULL)
      return NULL;
    return handler->js_stack_comparable_address_;
  }

  V8TryCatch(const V8TryCatch &) = delete;
  void operator=(const V8TryCatch &) = delete;
  void *operator new(size_t size) = delete;
  void operator delete(void *, size_t) = delete;

private:
  void ResetInternal();

  V8Isolate *isolate_;
  V8TryCatch *next_;
  //V8Value type
  V8Value *exception_; 
  //V8Message type
  V8Message *message_obj_;
  void *js_stack_comparable_address_;
  bool is_verbose_ : 1;
  bool can_continue_ : 1;
  bool capture_message_ : 1;
  bool rethrow_ : 1;
  bool has_terminated_ : 1;

  friend class V8Isolate;
  friend class v8::TryCatch;
};
class V8Context {
public:
  V8Context(V8Isolate *isolate, JSContext *ctx);
  V8Context(const V8Context &other);

  JSContext *ctx() { return m_ctx; }
  V8Isolate *isolate() { return m_isolate; }
  void enterTryCatch(qjsv8::V8TryCatch *try_catch) {
    try_catch->setNext(getTryCatch());
    m_try_stack.push_back(try_catch);
  }
  V8TryCatch *leaveTryCatch() {
    if (!m_try_stack.empty()) {
      V8TryCatch *top = m_try_stack.back();
      m_try_stack.pop_back();
      return top;
    }
    return nullptr;
  }
  void setException(qjsv8::V8Value*ex, qjsv8::V8Message*message) {
    if (!m_try_stack.empty()) {
      V8TryCatch *top = m_try_stack.back();
      top->setException(ex, message);
    }
  }
  V8TryCatch *getTryCatch() {
    if (!m_try_stack.empty()) {
      return m_try_stack.back();
    }
    return nullptr;
  }
  bool isCodeGenerationFromStringsAllowed() const {
    return m_isCodeGenerationFromStringsAllowed;
  }

private:
  friend class v8::Context;

  V8ObjectType m_type; // ����������ǰ�棬����ʶ������ʲô����
  JSContext *m_ctx;
  V8Isolate *m_isolate;
  std::vector<V8TryCatch *> m_try_stack;
  bool m_isCodeGenerationFromStringsAllowed;

  std::map<int, void *> m_alignedPointerInEmbedderDatas;
};

// �������麯���������һ���ֶβ���m_type
class V8Data {
public:
  V8Data(V8Context *ctx, JSValue value) {
    m_type = kObjectTypeData;
    m_ctx = ctx;
    m_qjsValue = value;
    m_nodeClassId = 0;
    m_nodeFlag = 0;
    m_nodeState = 0;

    v = V8Data_v;
    isObject = false;
    isExternal = false;
    isFunciton = false;
  }

  static JSValue V8Data_v(V8Data *self) { 
    return self->m_qjsValue; 
  }

  JSValue qjsValue() const{ return m_qjsValue; }

  JSContext *ctx() const {
    if (!m_ctx)
      return nullptr;
    return m_ctx->ctx();
  }

  V8Context *v8Ctx() const { return m_ctx; }

  V8ObjectType m_type;
  uint16_t m_nodeClassId;
  uint8_t m_nodeFlag;
  uint8_t m_nodeState;
protected:
  JSValue m_qjsValue;
  V8Context *m_ctx;

  typedef JSValue (*FN_v)(V8Data *self);

public:
  FN_v v;
  bool isObject;
  bool isExternal;
  bool isFunciton;
};

class V8Private : public V8Data {
public:
  V8Private(V8Context *ctx, JSValue value) : V8Data(ctx, value) {
    m_type = kObjectTypePrivate;
  }
};

class V8Value : public V8Data {
public:
  V8Value(V8Context *ctx, JSValue value) : V8Data(ctx, value) {
    m_type = kObjectTypeValue;
  }

  static V8Value *create(V8Context *ctx, JSValue value);
};

class V8Object : public V8Value {
public:
  static V8Object *create(V8Context *ctx, JSValue value);
  ~V8Object();

  static JSClassID getClassId() { return s_classId; }

private:
  V8Object(V8Context *ctx, JSValue value);

  friend class v8::Object;
  friend class v8::ObjectTemplate;
  friend class v8::FunctionTemplate;
  friend class v8::Function;
  friend class V8Array;
  friend class V8Function;
  friend class V8Function;
  friend class V8Template;

  friend JSValue wrapGetter(JSContext *ctx, JSValueConst thisVal,
                            void *userdata);
  friend JSValue wrapSetter(JSContext *ctx, JSValueConst this_val,
                            JSValueConst val, void *userdata);

  struct AccessorData {
    std::string name;
    v8::AccessorGetterCallback getter;
    v8::AccessorSetterCallback setter;
    V8Value *data;
  };

  static JSClassID s_classId;
  std::vector<JSCFunctionListEntry> *m_props; // ������������
  std::map<uint32_t, AccessorData *> m_accessorMap;

  int m_internalFieldCount;
  std::map<int, void *> m_alignedPointerInInternalFields;
};

class V8Primitive : public V8Value {
public:
  V8Primitive(V8Context *ctx, JSValue value) : V8Value(ctx, value) {
    m_type = kObjectTypePrimitive;
  }
};

class V8Name : public V8Primitive {
public:
  V8Name(V8Context *ctx, JSValue value) : V8Primitive(ctx, value) {
    m_type = kObjectTypeName;
  }
};

class V8Boolean : public V8Primitive {
public:
  V8Boolean(V8Context *ctx, JSValue value) : V8Primitive(ctx, value) {
    m_type = kObjectTypeBoolean;
  }
};
class V8String : public V8Name {
public:
  V8String(V8Context *ctx, JSValue value) : V8Name(ctx, value) {
    m_type = kObjectTypeString;
    v = V8String_v;
    m_externalString = nullptr;
    m_contextual= true;
    m_str = "";
  }
  V8String(V8Context *ctx, const std::string &str) : V8String(ctx, JS_NewString(ctx->ctx(), str.c_str())) {
    // m_type = kObjectTypeString;
    // v = V8String_v;
    // m_contextual= false;
    // m_str = str;
    // m_externalString = nullptr;
    // ensureQjsVal(ctx);
  }

  std::string getStr();

  const char*  c_str();
  static JSValue V8String_v(V8Data *self);

  void ensureQjsVal(V8Context *ctx);

protected:
  friend class v8::String;
  std::string m_str;
  bool m_contextual;
  v8::String::ExternalStringResourceBase *m_externalString;
};

class V8Symbol : public V8String { // ��ʱ�趨Ϊstring������
public:
  V8Symbol(V8Context *ctx, JSValue value) : V8String(ctx, value) {
    m_type = kObjectTypeSymbol;
    v = V8String_v;
  }
  V8Symbol(const std::string &str) : V8String(nullptr, JS_NULL) {
    m_type = kObjectTypeSymbol;
    m_str = str;
    v = V8String_v;
  }
};

class V8AccessorSignature : public V8Data {
public:
  V8AccessorSignature(V8Context *ctx, JSValue value) : V8Data(ctx, value) {
    m_type = kObjectTypeAccessorSignature;
  }
};

class V8Array : public V8Object {
public:
  V8Array(V8Context *ctx, JSValue value) : V8Object(ctx, value) {
    m_type = kObjectTypeArray;
  }
};

class V8Function : public V8Object {
public:
  V8Function(V8Context *ctx, v8::FunctionCallback callback, void *data)
      : V8Object(ctx, JS_NULL) {
    m_type = kObjectTypeFunction;
    m_callback = callback;
    isFunciton = true;
    m_data = data;
    v = V8Function_v;
  }

  // Ϊ���ӳٳ�ʼ��
  static JSValue V8Function_v(V8Data *self);
  std::string getName();
  void setName(const std::string &name);

private:
  friend class v8::Function;
  static JSValue onV8FunctionWrap(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv, void *userdata);

  v8::FunctionCallback m_callback;
  void *m_data;
  std::string m_name;
};

class V8External : public V8Value {
public:
  V8External(void *userdata) : V8Value(nullptr, JS_NULL) {
    m_type = kObjectTypeExternal;
    m_userdata = userdata;
    isExternal = true;
  }

  void *getUserdata() { return m_userdata; }

private:
  friend class v8::External;
  void *m_userdata;
};

class V8Template {
public:
  V8Template() {
    m_type = kObjectTypeTemplate;
    m_isInit = false;
    m_internalFieldCount = 0;
    m_parentTemplate = nullptr;
  }

  V8ObjectType m_type;

  struct Accessor {
    std::string name;
    v8::AccessorGetterCallback getter;
    v8::AccessorSetterCallback setter;

    v8::AccessorNameGetterCallback nameGetter;
    v8::AccessorNameSetterCallback nameSetter;
    void *data;
    Accessor() {
      getter = nullptr;
      setter = nullptr;
      nameGetter = nullptr;
      nameSetter = nullptr;
      data = nullptr;
    }
  };
  void newTemplateInstance(qjsv8::V8Context *ctx, qjsv8::V8Object *obj);

protected:
  friend class v8::ObjectTemplate;
  friend class v8::FunctionTemplate;
  friend class v8::Template;

  std::map<std::string, Accessor> m_accessors;

  int m_internalFieldCount;
  V8Template *m_parentTemplate;
  std::map<std::string, JSValue> m_props;
  bool m_isInit;
};

class V8ObjectTemplate : public V8Template {
public:
  V8ObjectTemplate() {
    printDebug("V8ObjectTemplate: %p\n", this);

    m_type = kObjectTypeObjectTemplate;
  }

private:
  friend class v8::ObjectTemplate;
  friend class v8::FunctionTemplate;
  friend class V8FunctionTemplate;
};

class V8FunctionTemplate : public V8Template {
public:
  V8FunctionTemplate() {
    // printDebug("V8FunctionTemplate: %p\n", this);
    m_type = kObjectTypeFunctionTemplate;
    m_userdata = nullptr;
    m_instanceTemplate = nullptr;
    m_prototypeTemplate = nullptr;
    m_argLength = 0;
  }

private:
  std::string getClassName();
  void setClassName(const std::string &name);

  v8::FunctionCallback m_constructor;
  void *m_userdata;
  std::string m_name;
  int m_argLength;
  V8ObjectTemplate *m_instanceTemplate;
  V8ObjectTemplate *m_prototypeTemplate;

  friend class v8::Template;
  friend class v8::ObjectTemplate;
  friend class v8::FunctionTemplate;
  friend class V8ObjectTemplate;
  static void
  functionTemplateCall(const v8::FunctionCallbackInfo<v8::Value> &info);
};

class V8Script {
public:
  void setSource(const std::string &src) { m_srcStr = src; }
  void setOrigin(const v8::ScriptOrigin *origin) { m_origin = origin; }
  const v8::ScriptOrigin *getOrigin() { return m_origin; }
  const std::string &getSource() const { return m_srcStr; }

  void setSourceName(const std::string &name) { m_srcName = name; }
  const std::string &getSourceName() const { return m_srcName; }

private:
  std::string m_srcStr;
  std::string m_srcName;
  const v8::ScriptOrigin *m_origin;
};

inline std::string getStringFromV8String(v8::Local<v8::String> str) {
  printEmptyFuncInfo(__FUNCTION__, true);

  qjsv8::V8String *v8string =
      v8::Utils::openHandle<v8::String, qjsv8::V8String>(*str);

  //     qjsv8::V8Isolate* isolate = qjsv8::V8Isolate::GetCurrent();
  //     JSContext* ctx = v8string->ctx();
  //     if (!ctx)
  //         ctx = isolate->getCurrentCtx()->ctx();
  //
  //     size_t len = 0;
  //     const char* nameString = JS_ToCStringLen(ctx, &len,
  //     v8string->v(v8string)); std::string ret(nameString);
  //     JS_FreeCString(v8string->ctx(), nameString);
  //
  //     return ret;
  return v8string->getStr();
}

class V8ArrayBuffer : public V8Value {
public:
  V8ArrayBuffer(V8Context *ctx, JSValue value) : V8Value(ctx, value) {
    m_type = kObjectTypeArrayBuffer;
  }
};

class V8Int16Array : public V8ArrayBuffer {
public:
  V8Int16Array(V8Context *ctx, JSValue value) : V8ArrayBuffer(ctx, value) {
    m_type = kObjectTypeInt16Array;
  }
};

class V8Int32Array : public V8ArrayBuffer {
public:
  V8Int32Array(V8Context *ctx, JSValue value) : V8ArrayBuffer(ctx, value) {
    m_type = kObjectTypeInt32Array;
  }
};

class V8Int8Array : public V8ArrayBuffer {
public:
  V8Int8Array(V8Context *ctx, JSValue value) : V8ArrayBuffer(ctx, value) {
    m_type = kObjectTypeInt8Array;
  }
};

class V8Int32 : public V8Value {
public:
  V8Int32(V8Context *ctx, JSValue value) : V8Value(ctx, value) {
    m_type = kObjectTypeInt32;
  }
};

class V8Integer : public V8Value {
public:
  V8Integer(V8Context *ctx, JSValue value) : V8Value(ctx, value) {
    m_type = kObjectTypeInteger;
  }
};

/**
 * An error message.
 */
class V8_EXPORT V8Message {
public:
  V8String *Get() const;

  V8_WARN_UNUSED_RESULT V8String *GetSourceLine(V8Context *context) const;

  /**
   * Returns the origin for the script from where the function causing the
   * error originates.
   */
  const v8::ScriptOrigin *GetScriptOrigin() const;

  /**
   * Returns the resource name for the script from where the function causing
   * the error originates.
   */
  v8::Local<V8Value> GetScriptResourceName() const;

  /**
   * Exception stack trace. By default stack traces are not captured for
   * uncaught exceptions. SetCaptureStackTraceForUncaughtExceptions allows
   * to change this option.
   */
  v8::Local<V8StackTrace> GetStackTrace() const;

  /**
   * Returns the number, 1-based, of the line where the error occurred.
   */
  V8_DEPRECATE_SOON("Use maybe version", int GetLineNumber() const);
  V8_WARN_UNUSED_RESULT v8::Maybe<int>
  GetLineNumber(v8::Local<V8Context> context) const;

  /**
   * Returns the index within the script of the first character where
   * the error occurred.
   */
  int GetStartPosition() const;

  /**
   * Returns the index within the script of the last character where
   * the error occurred.
   */
  int GetEndPosition() const;

  /**
   * Returns the error level of the message.
   */
  int ErrorLevel() const;

  /**
   * Returns the index within the line of the first character where
   * the error occurred.
   */
  V8_DEPRECATE_SOON("Use maybe version", int GetStartColumn() const);
  V8_WARN_UNUSED_RESULT v8::Maybe<int>
  GetStartColumn(v8::Local<V8Context> context) const;

  /**
   * Returns the index within the line of the last character where
   * the error occurred.
   */
  V8_DEPRECATED("Use maybe version", int GetEndColumn() const);
  V8_WARN_UNUSED_RESULT v8::Maybe<int>
  GetEndColumn(v8::Local<V8Context> context) const;

  /**
   * Passes on the value set by the embedder when it fed the script from which
   * this Message was generated to V8.
   */
  bool IsSharedCrossOrigin() const;
  bool IsOpaque() const;

  // TODO(1245381): Print to a string instead of on a FILE.
  static void PrintCurrentStackTrace(V8Isolate *isolate, FILE *out);

  // prop = [stack,message,...]
  // JS_FreeCString required.
  const char *GetPropertyCString(const char *prop) const {
    JSValue val = JS_GetPropertyStr(error_ctx_, error_, prop);
    if (!JS_IsUndefined(val)) {
      const char *stack = JS_ToCString(error_ctx_, val);
      // printDebug("%s\n", stack);
      // JS_FreeCString(ctx, stack);
      return stack;
    }
    JS_FreeValue(error_ctx_, val);
    return "";
  }

    const char *GetCString() const {
    if (!JS_IsUndefined(error_)) {
      const char *stack = JS_ToCString(error_ctx_, error_);
      // printDebug("%s\n", stack);
      // JS_FreeCString(ctx, stack);
      return stack;
    }
    return "";
  }

  JSValue GetProperty(const char *prop) const {
    JSValue val = JS_GetPropertyStr(error_ctx_, error_, prop);

    // JS_FreeValue(ctx, val);
    return val;
  }

  static const int kNoLineNumberInfo = 0;
  static const int kNoColumnInfo = 0;
  static const int kNoScriptIdInfo = 0;
  int start_position_ = 0;
  int start_column_ = 0;
  int end_position_ = 0;
  int line_number_ = 0;
  int end_column_ = 0;
  const v8::ScriptOrigin *script_origin_ = nullptr;
  JSContext *error_ctx_ = nullptr;
  JSValue error_;
  JSValue stack_;
  JSValue message_;
};

} // namespace qjsv8

//////////////////////////////////////////////////////////////////////////

namespace v8 {

template <class From, class To>
inline To *Utils::openHandle(const From *that, bool allowEmptyHandle) {
  if (!that)
    return nullptr;
  return *(To **)that;
}

template <class From, class To> inline v8::Local<To> Utils::convert(From *obj) {
  // return Local<To>(reinterpret_cast<To*>(obj));
  if (!obj)
    return Local<To>();
  qjsv8::V8Isolate *isolate = qjsv8::V8Isolate::GetCurrent();
  void **ret = isolate->findHandleScopeEmptyIndex();
  *ret = obj;
  To *that = reinterpret_cast<To *>(ret);
  return Local<To>(that);
}

// 使用宏定义生成具体的类型转换函数
#define IMPL_ALLOW_TYPE_AS(className)                                          \
  DECL_ALLOW_TYPE_AS(className)                                                \
  inline qjsv8::V8##className *fromV8(const v8::className *that) {             \
    return v8::Utils::openHandle<v8::className, qjsv8::V8##className>(that);   \
  }                                                                            \
  inline qjsv8::V8##className *fromV8Local(v8::Local<v8::className> *that) {   \
    return v8::Utils::openHandle<v8::Local<v8::className>,                     \
                                 qjsv8::V8##className>(that);                  \
  }                                                                            \
  inline qjsv8::V8##className *fromV8##className(void *that) {                 \
    return v8::Utils::openHandle<v8::className, qjsv8::V8##className>(         \
        (v8::className *)that);                                                \
  }                                                                            \
  inline v8::Local<v8::className> toV8Local(qjsv8::V8##className *that) {      \
    return v8::Utils::convert<qjsv8::V8##className, v8::className>(that);      \
  }                                                                            \
  inline v8::Local<v8::className> toV8##className(qjsv8::V8Data *that) {       \
    return v8::Utils::convert<qjsv8::V8Data, v8::className>(that);             \
  }                                                                            \
  inline v8::Local<v8::className> toV8##className(void *that) {                \
    return v8::Utils::convert<qjsv8::V8##className, v8::className>(            \
        (qjsv8::V8##className *)that);                                         \
  }

IMPL_ALLOW_TYPE_AS(Isolate)
IMPL_ALLOW_TYPE_AS(Primitive)
IMPL_ALLOW_TYPE_AS(Message)
IMPL_ALLOW_TYPE_AS(Private)
IMPL_ALLOW_TYPE_AS(StackTrace)
IMPL_ALLOW_TYPE_AS(Script)
IMPL_ALLOW_TYPE_AS(Value)
IMPL_ALLOW_TYPE_AS(TryCatch)
IMPL_ALLOW_TYPE_AS(Function)
IMPL_ALLOW_TYPE_AS(Context)
IMPL_ALLOW_TYPE_AS(String)
IMPL_ALLOW_TYPE_AS(Symbol)
IMPL_ALLOW_TYPE_AS(Integer)
IMPL_ALLOW_TYPE_AS(Boolean)
IMPL_ALLOW_TYPE_AS(Int32)
IMPL_ALLOW_TYPE_AS(Object)
IMPL_ALLOW_TYPE_AS(Array)
IMPL_ALLOW_TYPE_AS(ArrayBuffer)
IMPL_ALLOW_TYPE_AS(External)
IMPL_ALLOW_TYPE_AS(ObjectTemplate)
IMPL_ALLOW_TYPE_AS(Int8Array)
IMPL_ALLOW_TYPE_AS(Int16Array)
IMPL_ALLOW_TYPE_AS(Int32Array)
IMPL_ALLOW_TYPE_AS(FunctionTemplate)

} // namespace v8
#endif