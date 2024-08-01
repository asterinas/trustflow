use clap::Parser;
use std::fs;
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(name = "generator")]
#[command(long_about = None)]
pub struct Args {
    /// Directory containing the header files
    #[arg(long, value_name = "DIR")]
    headers_dir: PathBuf,

    #[arg(long, value_name = "FILE")]
    header: String,

    /// Directory to write the generated code to
    #[arg(short, long, value_name = "DIR")]
    output: PathBuf,
}

fn main() {
    let args = Args::parse();

    println!("{:?}", fs::canonicalize(&args.headers_dir).unwrap());
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header(args.header)
        //.clang_arg("-I/home/huanxian/code/trustflow")
        .clang_arg(format!(
            "-I{}",
            fs::canonicalize(&args.headers_dir)
                .unwrap()
                .to_str()
                .unwrap()
        ))
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    bindings
        .write_to_file(args.output.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
