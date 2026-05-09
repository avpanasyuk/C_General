/**
@file
@brief defimes the rest of comparison operators if == and > are defined
*/

#pragma once

template <class T>
inline bool operator!=(const T& lhs, const T& rhs) {
    return !(lhs == rhs);
}

template <class T>
inline bool operator<=(const T& lhs, const T& rhs) {
    return !(lhs > rhs);
}

template <class T>
inline bool operator<(const T& lhs, const T& rhs) {
    return rhs > lhs;
}

template <class T>
inline bool operator>=(const T& lhs, const T& rhs) {
    return !(lhs < rhs);
}
