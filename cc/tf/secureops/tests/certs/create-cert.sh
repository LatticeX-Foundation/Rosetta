mkdir certs
# generate private key
openssl genrsa -out certs/server-prikey 4096
# if ~/.rnd not exists, generate it with `openssl rand`
if [ ! -f "${HOME}/.rnd" ]; then openssl rand -writerand ${HOME}/.rnd; fi
# generate sign request
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Rosetta SSL IO server/OU=Rosetta server unit/CN=server' -key certs/server-prikey -out certs/cert.req
# sign certificate with cert.req
openssl x509 -req -days 365 -in certs/cert.req -signkey certs/server-prikey -out certs/server-nopass.cert
