#ifndef LOG_HPP
#define	LOG_HPP
#include <iostream>

#ifdef VDBG
#define DAS_DBG(c) {c}
#define DAS_DBG_NO_SCOPE(c) c
#define DAS_LOG_DBG(s) std::cerr << s << std::endl;
#else
#define DAS_DBG(c)
#define DAS_DBG_NO_SCOPE(c)
#define DAS_LOG_DBG(s)
#endif



#endif	/* LOG_HPP */

