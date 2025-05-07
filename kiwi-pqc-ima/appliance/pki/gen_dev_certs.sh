#!/bin/bash

gen_cert_algo () {
	algo=$1

	if [ -d "${algo}" ]; then
		echo "${algo} already exists. skip."
		return
	fi

	mkdir ${algo}
	cd ${algo}

	openssl req \
		-x509 \
		-sha256 \
		-newkey ${algo} \
		-keyout ${algo}_key.pem \
		-out ${algo}_cert.pem \
		-days 365 \
		-nodes \
		-subj "/CN=${algo}.test/O=${algo} Test Certificate/C=DE"
	openssl x509 \
		-in ${algo}_cert.pem \
		-outform DER \
		-out ${algo}_cert.crt
	openssl x509 \
		-in ${algo}_cert.crt \
		-inform DER \
		-noout \
		-pubkey \
		-out ${algo}_pub.pem
	openssl asn1parse \
		-in ${algo}_cert.pem \
		-strparse 4 \
		-out ${algo}_cert_body.bin
	openssl x509 \
		-in ${algo}_cert.crt \
		-text \
		-noout \
		-certopt ca_default \
		-certopt no_validity \
		-certopt no_serial \
		-certopt no_subject \
		-certopt no_extensions \
		-certopt no_signame  | grep -v 'Signature' | tr -d '[:space:]:' | xxd -r -p > ${algo}_cert_signature.bin
	openssl dgst \
		-sha256 \
		-verify ${algo}_pub.pem \
		-signature ${algo}_cert_signature.bin \
		${algo}_cert_body.bin

	cd ..
}

gen_cert_algo "$1"

#gen_cert_algo "dilithium2"
#gen_cert_algo "dilithium3"
#gen_cert_algo "dilithium5"

#gen_cert_algo "falcon512"
#gen_cert_algo "falcon1024"

#gen_cert_algo "sphincssha2128fsimple"
#gen_cert_algo "sphincssha2128ssimple"
#gen_cert_algo "sphincssha2192fsimple"
#gen_cert_algo "sphincssha2192ssimple"
#gen_cert_algo "sphincssha2256fsimple"
#gen_cert_algo "sphincssha2256ssimple"

#gen_cert_algo "mldsa44"
#gen_cert_algo "mldsa65"
#gen_cert_algo "mldsa87"
