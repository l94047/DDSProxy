# DDS ROUTER SECURITY RESOURCES

This directory contains several sample files needed to implement secure **TLS over TCP** communication.
These files are required to configure the TCP transport protocol with TLS in Fast DDS.

> :warning: Do not use these files in a real scenario. Generate your own certificates and parameters.

## COMMANDS

Following are the commands used to generate this example's keys and certificates

### Certification Authority (CA)

```sh
# Generate the Certificate Authority (CA) Private Key > ca.key
openssl ecparam -name prime256v1 -genkey -noout -out ca.key
# openssl ecparam -name prime256v1 -genkey | openssl ec -aes256 -out ca.key -passout pass:cakey # with password

# Generate the Certificate Authority Certificate > ca.crt
openssl req -new -x509 -sha256 -key ca.key -out ca.crt -days 365 -config ca.cnf
# openssl req -new -x509 -sha256 -key ca.key -out ca.crt -days 365 -config ca.cnf -passin pass:cakey # with password
```

### DDS-Router Certificate

```sh
# Generate the DDS-Router Certificate Private Key > ddsproxy.key
openssl ecparam -name prime256v1 -genkey -noout -out ddsproxy.key
# openssl ecparam -name prime256v1 -genkey | openssl ec -aes256 -out ddsproxy.key -passout pass:ddsproxypass # with password

# Generate the DDS-Router Certificate Signing Request  > ddsproxy.csr
openssl req -new -sha256 -key ddsproxy.key -out ddsproxy.csr -config ddsproxy.cnf
# openssl req -new -sha256 -key ddsproxy.key -out ddsproxy.csr -config ddsproxy.cnf -passin pass:ddsproxypass # with password

# Generate the DDS-Router Certificate (computed on the CA side) > ddsproxy.crt
openssl x509 -req -in ddsproxy.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out ddsproxy.crt -days 1000 -sha256
# openssl x509 -req -in ddsproxy.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out ddsproxy.crt -days 1000 -sha256 -passin pass:cakey # with password
```

### DH PARAMETERS

```sh
# Generate the Diffie-Hellman (DF) parameters to define how OpenSSL performs the DF key-exchange > dh_params.pem
openssl dhparam -out dh_params.pem 2048
```

## Use

```yaml
# Use this tag inside a wan participant configuration
tls:
  ca: "ca.crt"
  password: ""
  private_key: "ddsproxy.key"
  cert: "ddsproxy.crt"
  dh_params: "dh_params.pem"
```
