# this is a bit heavy on the regex, may be slow on old-old systems.
path /sbin:/usr/sbin:/bin:/usr/bin:<env>
base cyan
ifarg --help:<none>
regex cyan+:default <.+>
regex cyan+:default --[^ ]+
ifarg -s
regex cyan+:default [0-9A-Fa-f]{8} [0-9A-Fa-f]{8} [0-9A-Fa-f]{8} [0-9A-Fa-f]{8}
ifarg -R
regex cyan+:default ^[0-9A-Fa-f]{8,16}[ ]
regex cyan+:default [ ](_{1,2})[A-Za-z0-9_]+$
ifnarg -s:-R
regex cyan+:default ^[ ]*[A-Za-z0-9\/]+:
ifnarg --help:<none>:-s:-R
regex cyan+:default 0x[0-9A-Fa-f]{1,16}
regex cyan+:default [\<\>\(\)\*\+\$\%\,\.]