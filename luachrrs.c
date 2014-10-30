#include <lua.h>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <lauxlib.h>
#include <crypt.h>
#include <stdlib.h>
#include <cconv.h>

#define cm_memzero(buf, n) (void) memset(buf, 0, n)
#define cm_memset(buf, c, n) (void) memset(buf, c, n)

#define GENERATE_SEED() (((long) (time(0) * getpid())) ^ ((long)1000000.0))
#define CRYPT_RAND get_rand()
#define MIN(m, n)  ((m) < (n)) ? (m) : (n)
#define STD_DES_CRYPT 1
#define MD5_CRYPT 1

#if STD_DES_CRYPT
#define MAX_SALT_LEN 2
#endif

#if EXT_DES_CRYPT
#undef MAX_SALT_LEN
#define MAX_SALT_LEN 9
#endif

#if MD5_CRYPT
#undef MAX_SALT_LEN
#define MAX_SALT_LEN 12
#endif


static unsigned char itoa64[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void _to64(char *s, long v, int n)
{
  while (--n >= 0) {
		*s++ = itoa64[v&0x3f];
		v >>= 6;
	}

}

static long get_rand() {
	long ret;
	srand((unsigned int) GENERATE_SEED());
	ret = rand();
	return ret;
}

static int ccrypt(lua_State *L)
{
	char salt[MAX_SALT_LEN + 1];
	char *crypt_res;
	const char *str, *salt_in = NULL;
	size_t salt_in_len = 0;
	
	salt[0] = salt[MAX_SALT_LEN] = '\0';

    if(lua_gettop(L) < 1)
    {
        lua_pushstring(L, "Bad argument number");
        lua_error(L);
        return 1;
    }

    if((str = lua_tostring(L, 1)) == NULL)
    {
        lua_pushstring(L, "Bad key");
        lua_error(L);
        return 1;
    }

    salt_in = lua_tolstring(L, 2, &salt_in_len);
	
    if (salt_in) {
		memcpy(salt, salt_in, MIN(MAX_SALT_LEN, salt_in_len));
	}
	
	if (!*salt) {
		#if MD5_CRYPT
		strncpy(salt, "$1$", MAX_SALT_LEN);
		_to64(&salt[3], CRYPT_RAND, 4);
		_to64(&salt[7], CRYPT_RAND, 4);
		strncpy(&salt[11], "$", MAX_SALT_LEN - 11);
		#elif STD_DES_CRYPT
		_to64(&salt[0], CRYPT_RAND, 2);
		salt[2] = '\0';
		#endif
		salt_in_len = strlen(salt);
	}
	else {
		salt_in_len = MIN(MAX_SALT_LEN, salt_in_len);
	}
	
	crypt_res = crypt(str, salt);

    lua_pushstring(L, crypt_res);
    return 1;
}

static int luacconv(lua_State *L)
{
	cconv_t cd;
	const char *fromcode, *tocode, *str;
	char *outstr, *p;
	size_t	inlen, outlen, size;

	if (lua_gettop(L) != 3)
	{
		lua_pushstring(L, "Bad argument number");
		lua_error(L);
		return 1;
	}

	fromcode = lua_tostring(L, 1);
	tocode = lua_tostring(L, 2);
	str = lua_tolstring(L, 3, &inlen);
	if((cd = cconv_open(tocode, fromcode)) == (cconv_t)(-1)) {
		lua_pushstring(L, str);
		return 1;
	}
	outlen = 3 * inlen;
	outstr = p = (char *)malloc(outlen);

	cm_memzero(outstr, outlen);

	size = cconv(cd, &str, &inlen, &p, &outlen);
	cconv_close(cd);
	if (size == (size_t)(-1))
	{
		lua_pushstring(L, str);
		free(outstr);
		return 1;
	}
	lua_pushstring(L, outstr);
	free(outstr);
	return 1;
}

int luaopen_chrrs(lua_State *L)
{
	luaL_Reg reg[] = {
		{ "cconv", luacconv },
		{ "crypt", ccrypt },
		{ NULL, NULL }
	};

	luaL_register(L, "chrrs", reg);
	return 1;
}
