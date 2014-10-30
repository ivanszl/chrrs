name
=====

chrrs - Lua library for Traditional Chinese and Simplified Chinese mutual conversion by the cconv,crypt the string

status
=====

This library is considered production ready.

Description
===========

This Lua library is a conversion and crypt ths string run on the lua:

This Lua library takes advantage of cconv API and crypt API

Synopsis
========

```lua
    # you do not need the following line if you are using
    # the ngx_openresty bundle:
    lua_package_path "/path/to/lua-resty-redis/lib/?.lua;;";

    server {
        location /test {
            content_by_lua '
                local chrrs = require "chrrs"
                ngx.say(chrrs.cconv("UTF8-TW", "UTF8-CN", ngx.var.arg_str))
                ngx.say(chrrs.crypt(ngx.var.arg_instr))
            ';
        }
    }
```

Author
======

Ivan Lam ivan.lin.1985@gmail.com.
