#ifndef OPTIONAL_HPP
#define	OPTIONAL_HPP
#include <boost/optional.hpp>

namespace das {

    template<typename T>
    class optional : public boost::optional<T> {
        typedef boost::optional<T> super;
    public:

        optional() : super() {
        }

        optional(const T& val) : super(val) {
        }

        optional(const boost::optional<T> &rhs) : super(rhs) {
        }

        const optional<T> &
                operator=(const boost::optional<T> &rhs) {
            super::operator=(rhs);
            return *this;
        }

        /* we do not return a reference because we won't allow to store references
         * due to the transient nature of this object
         */
        operator T() {
            return super::get();
        }
        
    };

    template <typename T>
    bool operator ==(const optional<T> &lhs, const optional<T> &rhs) {
        const boost::optional<T> &b_lhs = lhs;
        const boost::optional<T> &b_rhs = rhs;
        return b_lhs == b_rhs;
    }

    template <typename T>
    bool operator ==(const T &lhs, const optional<T> &rhs) {
        const boost::optional<T> &b_rhs = rhs;
        return lhs == b_rhs;
    }

    template <typename T>
    bool operator ==(const optional<T> &lhs, const T &rhs) {
        const boost::optional<T> &b_lhs = lhs;
        return b_lhs == rhs;
    }

    template <typename T>
    bool operator !=(const optional<T> &lhs, const optional<T> &rhs) {
        const boost::optional<T> &b_lhs = lhs;
        const boost::optional<T> &b_rhs = rhs;
        return b_lhs != b_rhs;
    }

    template <typename T>
    bool operator !=(const T &lhs, const optional<T> &rhs) {
        const boost::optional<T> &b_rhs = rhs;
        return lhs != b_rhs;
    }

    template <typename T>
    bool operator !=(const optional<T> &lhs, const T &rhs) {
        const boost::optional<T> &b_lhs = lhs;
        return b_lhs != rhs;
    }
}





#endif	/* OPTIONAL_HPP */

