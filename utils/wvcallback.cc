#include "wvcallback.h"

/* If we're not using TR1, we must be using Boost. */
#if !defined(HAVE_TR1_FUNCTIONAL) && defined(HAVE_BOOST_THROW_EXCEPTION_HPP)
#include <boost/throw_exception.hpp>
#include "wvcrash.h"

#ifdef BOOST_NO_EXCEPTIONS
/* std::tr1::function does this when operator() is called on an empty
 * object. This is a bit heavy-handed, but it's all I can think of for
 * now.  */
void boost::throw_exception(std::exception const &e)
{
    std::abort();
}
#endif

#endif
