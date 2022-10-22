#pragma once

template <typename T>
struct SmartPointer
{
   T *ptr;

   T* operator->() const
   {
      return ptr;
   }
   
   ~SmartPointer()
   {
      delete ptr;
   }
};

template <typename T>
SmartPointer<T> make_smart_pointer(T* t)
{
   return SmartPointer{
      .ptr = t
   };   
}
