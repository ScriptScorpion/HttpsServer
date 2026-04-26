#!/bin/sh

SCRIPT_DIR="$(dirname "$(realpath "$0")")"

# AUTOMATIC VERSION
# CERT_KEY="strongkey"
# openssl req -x509 -passout pass:$CERT_KEY -newkey rsa:4096 -keyout "$SCRIPT_DIR/key_enc.pem" -out "$SCRIPT_DIR/cert.pem" -sha256 -days 365 -batch
# openssl rsa -passin pass:$CERT_KEY -in "$SCRIPT_DIR/key_enc.pem" -out "$SCRIPT_DIR/key.pem"
# rm -f "$SCRIPT_DIR/key_enc.pem"

# MANUAL VERSION
openssl req -x509 -newkey rsa:4096 -keyout "$SCRIPT_DIR/key_enc.pem" -out "$SCRIPT_DIR/cert.pem" -sha256 -days 365 -batch
openssl rsa -in "$SCRIPT_DIR/key_enc.pem" -out "$SCRIPT_DIR/key.pem"
rm -f "$SCRIPT_DIR/key_enc.pem"
