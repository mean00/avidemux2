
#ifdef _MSC_VER
  #define ADM_D3D_CPP_API
#endif

#ifdef ADM_D3D_CPP_API
  #define EXPAND(x) x
  #define D3DCallNoArg(CLASS,call,obj) EXPAND(obj->call())
  #define D3DCall(CLASS,call,obj, ...) EXPAND(obj->call(__VA_ARGS__))
#else
  #define CINTERFACE
  #define COBJMACROS
  #define EXPAND(x) x
  #define D3DCallNoArg(CLASS,call,obj) EXPAND(CLASS##_##call(obj))
  #define D3DCall(CLASS,call,obj, ...) EXPAND(CLASS##_##call(obj,__VA_ARGS__))
#endif
