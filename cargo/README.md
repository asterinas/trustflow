# TrustFlow Attestation Rust Library Binding


Excecute this command to generate the binding code:
```
cargo run -p trustflow-attestation-generator -- \
    --headers-dir .. --header trustflow-attestation-generator/src/bindings.h  -o trustflow-attestation-sys/src
```