# Trustedflow Attestation Rust Library Binding


Excecute this command to generate the binding code:
```
cargo run -p trustedflow-attestation-generator -- \
    --headers-dir .. --header trustedflow-attestation-generator/src/bindings.h  -o trustedflow-attestation-sys/src
```