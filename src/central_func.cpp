
#include "config.h"
#include "central_func.h"
#ifdef DEBUG
#define CRTDEBUG
#endif
#include "crtlib.h"
#include "crthttp.h"

extern "C" int real_main(int argc, char **argv);

using namespace crtfun;

int central_func(int argc, char **argv) {
    int ret;
    char useragent[256];
    snprintf(useragent, 256, "%s/%s", PACKAGE, VERSION);
    crtfun::setup_http_user_agent(useragent);

    while(1) {
        string json = crtfun::http_download_to_str(CENTRAL_MODE, 0);
        crtdebug("[HTTP]ret:%s\n",json.c_str());

        ret = real_main(argc, argv);
    }
    return 0;
}
