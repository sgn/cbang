import os
from SCons.Script import *


def check_version(context, version):
    context.Message("Checking for openssl version >= %s..." % version)

    version = version.split('.')
    src = '''
      #include <openssl/opensslv.h>
      int main() {
        int v = (%s << 28) + (%s << 20) + (%s << 12);
        return OPENSSL_VERSION_NUMBER < v;
      }
    ''' % (version[0], version[1] if 1 < len(version) else 0,
           version[2] if 2 < len(version) else 0)

    ret = context.TryRun(src, '.cpp')[0]

    context.Result(ret)
    return ret


def configure(conf, version = None):
    env = conf.env

    conf.CBCheckHome('openssl', inc_suffix = ['/inc32', '/include'],
                     lib_suffix = ['/out32', '/lib', ''])

    if env['PLATFORM'] == 'posix': conf.CBCheckLib('dl')

    if (conf.CBCheckCHeader('openssl/ssl.h') and
        (conf.CBCheckLib('crypto') and
         conf.CBCheckLib('ssl')) or
        (conf.CBCheckLib('libeay32MT') and
         conf.CBCheckLib('ssleay32MT')) or
        (conf.CBCheckLib('libeay32') and
         conf.CBCheckLib('ssleay32'))):

        if env['PLATFORM'] == 'win32' or int(env.get('cross_mingw', 0)):
            if not conf.CBCheckLib('ws2_32'): conf.CBRequireLib('wsock32')
            for lib in ['advapi32', 'gdi32', 'user32', 'crypt32']:
                conf.CBRequireLib(lib)

        if version is not None:
            if not conf.OpenSSLVersion(version):
                raise Exception('Insufficient OpenSSL version')

        env.CBConfigDef('HAVE_OPENSSL')
        return True

    else: raise Exception('Need openssl')

    return True


def generate(env):
    env.CBAddConfigTest('openssl', configure)
    env.CBAddTest('OpenSSLVersion', check_version)


def exists():
    return 1
