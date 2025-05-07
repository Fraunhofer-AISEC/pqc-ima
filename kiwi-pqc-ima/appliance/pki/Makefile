TARGETS = \
	rsa-4096 \
	mldsa44 \
	mldsa65 \
	mldsa87 \
	falcon512 \
	falcon1024 \
	sphincssha2128fsimple \
	sphincssha2128ssimple \
	sphincssha2192fsimple \
	sphincssha2192ssimple \
	sphincssha2256fsimple \
	sphincssha2256ssimple \
	sphincsshake128fsimple \
	sphincsshake128ssimple \
	sphincsshake192fsimple \
	sphincsshake192ssimple \
	sphincsshake256fsimple \
	sphincsshake256ssimple \

all: $(TARGETS) prime256v1

$(TARGETS) prime256v1: export OPENSSL_CONF = openssl-ca.cnf

$(TARGETS): rootca
	mkdir -p $@
	openssl req \
		-new \
		-newkey `echo $@ | tr "-" ":"` \
		-keyout $@/$@.key \
		-out $@/$@.csr \
		-nodes \
		-subj "/CN=$@.test/O=$@ Test Certificate/C=DE"
	openssl ca -batch -days 3560 \
		-keyfile rootca/rootca.key \
		-cert rootca/rootca_crt.pem \
		-policy policy_anything \
		-notext \
		-out $@/$@_crt.pem \
		-infiles $@/$@.csr
	openssl x509 \
		-in $@/$@_crt.pem \
		-outform DER \
		-out $@/$@_crt.der

prime256v1: rootca
	mkdir -p $@
	openssl ecparam -out $@/$@.param -name $@
	openssl req \
		-new \
		-newkey ec:$@/$@.param \
		-keyout $@/$@.key \
		-out $@/$@.csr \
		-nodes \
		-subj "/CN=$@.test/O=$@ Test Certificate/C=DE"
	openssl ca -batch -days 3560 \
		-keyfile rootca/rootca.key \
		-cert rootca/rootca_crt.pem \
		-policy policy_anything \
		-notext \
		-out $@/$@_crt.pem \
		-infiles $@/$@.csr
	openssl x509 \
		-in $@/$@_crt.pem \
		-outform DER \
		-out $@/$@_crt.der

hsslms: rootca
	mkdir -p $@
	botan keygen \
		--algo=HSS-LMS \
		--params='SHA-256,HW(10,8),HW(10,8)' > $@/$@.key
	botan gen_pkcs10 $@/$@.key CN \
		--country=DE \
		--dns=example.org \
		--organization=AISEC \
		--email=foo@bar \
		--path-limit=1 > $@/$@.csr
	botan sign_cert \
		--duration=3650 \
		rootca/rootca_crt.pem \
		rootca/rootca.key \
		$@/$@.csr > $@/$@_crt.pem
	openssl x509 \
		-in $@/$@_crt.pem \
		-outform DER \
		-out $@/$@_crt.der

rootca:
	mkdir -p rootca/newcerts
	touch rootca/index.txt
	echo '01' > rootca/serial
	openssl req -x509 -new -newkey rsa:4096 -keyout rootca/rootca.key -out rootca/rootca_crt.pem -subj "/CN=rootca" -nodes
	#openssl x509 -in rootca/rootca_crt.pem -outform DER -out rootca/rootca_crt.der

clean:
	rm -rf $(TARGETS) secp256k1 rootca
