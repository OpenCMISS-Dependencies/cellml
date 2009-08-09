#ifndef _UTILS_HXX_
#define _UTILS_HXX_

#define RETURN_INTO_WSTRING(lhs, rhs) \
  wchar_t* tmp_##lhs = rhs;           \
  std::wstring lhs;                   \
  if (tmp_##lhs)                      \
  {                                   \
    lhs = std::wstring(tmp_##lhs);    \
    free(tmp_##lhs);                  \
  }                                 

#define RETURN_INTO_STRING(lhs, rhs)  \
  char* tmp_##lhs = rhs;              \
  std::string lhs;                    \
  if (tmp_##lhs)                      \
  {                                   \
    lhs = std::string(tmp_##lhs);     \
    free(tmp_##lhs);                  \
  }

#define QUERY_INTERFACE(lhs, rhs, type)                               \
  if (rhs != NULL)                                                    \
  {                                                                   \
    void* _qicast_obj = rhs->query_interface(#type);                  \
    if (_qicast_obj != NULL)                                          \
    {                                                                 \
      lhs = already_AddRefd<iface::type>(reinterpret_cast<iface::type*>(_qicast_obj)); \
    }                                                                   \
    else                                                                \
      lhs = NULL;                                                       \
  }                                                                     \
  else lhs = NULL;

#define QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  QUERY_INTERFACE(lhs, rhs, type);              \
  if (rhs != NULL)                              \
  {                                             \
    rhs->release_ref();                         \
    rhs = NULL;                                 \
  }

#define DECLARE_QUERY_INTERFACE(lhs, rhs, type) \
  iface::type* lhs;                             \
  QUERY_INTERFACE(lhs, rhs, type)

#define DECLARE_QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  iface::type* lhs;                                     \
  QUERY_INTERFACE_REPLACE(lhs, rhs, type)

#define CELLML_TO_VARIABLE_INTERFACE( cellml, variable)                 \
  if (cellml == iface::cellml_api::INTERFACE_IN) variable(Variable::IN); \
  else if (cellml == iface::cellml_api::INTERFACE_OUT)                  \
    variable(Variable::OUT);                                              \
  else if (cellml == iface::cellml_api::INTERFACE_NONE)                 \
    variable(Variable::NONE);                                             \
  else ERROR("CELLML_TO_VARIABLE_INTERFACE","Invalid variable interface\n")

template<class T>
class already_AddRefd
{
public:
  already_AddRefd(T* aPtr)
    : mPtr(aPtr)
  {
  }

  ~already_AddRefd()
  {
  }

  operator T*() const
  {
    return mPtr;
  }

  T* getPointer() const
  {
    return mPtr;
  }
private:
  T* mPtr;
};

template<class T>
class ObjRef
{
public:
  ObjRef()
    : mPtr(NULL)
  {
  }

  ObjRef(const ObjRef<T>& aPtr)
  {
    mPtr = aPtr.getPointer();
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  ObjRef(T* aPtr)
    : mPtr(aPtr)
  {
    mPtr->add_ref();
  }

  ObjRef(const already_AddRefd<T> aar)
  {
    mPtr = aar.getPointer();
  }

  ~ObjRef()
  {
    if (mPtr != NULL)
      mPtr->release_ref();
  }

  T* operator-> () const
  {
    return mPtr;
  }

  T* getPointer() const
  {
    return mPtr;
  }

  operator T* () const
  {
    return mPtr;
  }

  void operator= (T* newAssign)
  {
    if (mPtr == newAssign)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = newAssign;
    if (newAssign != NULL)
      mPtr->add_ref();
  }

  // We need these explicit forms or the default overloads the templates below.
  void operator= (const already_AddRefd<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  void operator= (const ObjRef<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  template<class U>
  void operator= (const already_AddRefd<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  template<class U>
  void operator= (const ObjRef<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

private:
  T* mPtr;
};

template<class T, class U> bool
operator==(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() == rhs.getPointer());
}

template<class T, class U> bool
operator!=(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() != rhs.getPointer());
}

#define RETURN_INTO_OBJREF(lhs, type, rhs)  \
  ObjRef<type> lhs                          \
  (                                         \
    already_AddRefd<type>                   \
    (                                       \
      static_cast<type*>                    \
      (                                     \
        rhs                                 \
      )                                     \
    )                                       \
  )

#define RETURN_INTO_OBJREFD(lhs, type, rhs) \
  ObjRef<type> lhs \
  ( \
    already_AddRefd<type> \
    ( \
      dynamic_cast<type*> \
      ( \
        rhs \
      ) \
    )\
  )

#endif /* _UTILS_HXX_ */
