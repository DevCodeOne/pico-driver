#pragma once

namespace PicoDriver {

template<typename T>
struct MapToType;

template<typename T>
using MappedType = typename MapToType<T>::Type;

template<typename T>
using TagType = typename MapToType<T>::TagType;

}