// Copyright 2024 Ant Group Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use sdc_apis::secretflowapis::v2::sdc::UnifiedAttestationAttributes;
use std::ffi::CString;

const BUFFER_SIZE_IN_BYTES: u32 = 8192;

pub type Error = Box<dyn std::error::Error + Send + Sync>;

pub fn parse_attributes_from_report(
    report_json_str: &str,
) -> Result<UnifiedAttestationAttributes, Error> {
    let c_report_json_str = CString::new(report_json_str.as_bytes())
        .map_err(|_| Error::from("attestation_report_verify: report_json_str ffi failure"))?;

    let mut attrs_len = get_attributes_max_size()?;
    let mut attrs_buf = vec![0; attrs_len as usize];

    let mut msg_buf = vec![0; BUFFER_SIZE_IN_BYTES as usize];
    let mut msg_len: u32 = BUFFER_SIZE_IN_BYTES;
    let mut details_len: u32 = 0;

    // c lib interface
    let code = unsafe {
        trustflow_attestation_sys::ParseAttributesFromReport(
            c_report_json_str.as_ptr(),
            report_json_str.len() as u32,
            attrs_buf.as_mut_ptr() as *mut i8,
            &mut attrs_len,
            msg_buf.as_mut_ptr() as *mut i8,
            &mut msg_len,
            std::ptr::null_mut(),
            &mut details_len,
        )
    };
    if code != 0 {
        match std::str::from_utf8(&msg_buf[..(msg_len as usize)]) {
            Ok(v) => return Err(v.into()),
            Err(e) => return Err(format!("parse error msg failed, {}", e).into()),
        };
    }

    Ok(serde_json::from_slice(&attrs_buf[..(attrs_len as usize)])?)
}

pub fn get_attributes_max_size() -> Result<u32, Error> {
    return Ok(unsafe { trustflow_attestation_sys::GetAttributesMaxSize() });
}

/// Verifies the given attestation report against the given policy.
///
/// # Arguments
///
/// * `report_json_str` - The JSON string representation of the attestation report.
/// * `policy_json_str` - The JSON string representation of the policy.
///
/// # Returns
///
/// A result containing either an empty tuple indicating success or an error message.
pub fn attestation_report_verify(
    report_json_str: &str,
    policy_json_str: &str,
) -> Result<(), Error> {
    let c_report_json_str = CString::new(report_json_str.as_bytes())
        .map_err(|_| Error::from("attestation_report_verify: report_json_str ffi failure"))?;

    let c_policy_json_str = CString::new(policy_json_str.as_bytes())
        .map_err(|_| Error::from("attestation_report_verify: policy_json_str ffi failure"))?;

    let mut msg_buf = vec![0; BUFFER_SIZE_IN_BYTES as usize];
    let mut msg_len: u32 = BUFFER_SIZE_IN_BYTES;
    let mut details_len: u32 = 0;

    // c lib interface
    let code = unsafe {
        trustflow_attestation_sys::AttestationReportVerify(
            c_report_json_str.as_ptr(),
            report_json_str.len() as u32,
            c_policy_json_str.as_ptr(),
            policy_json_str.len() as u32,
            msg_buf.as_mut_ptr() as *mut i8,
            &mut msg_len,
            std::ptr::null_mut(),
            &mut details_len,
        )
    };
    if code != 0 {
        match std::str::from_utf8(&msg_buf[..(msg_len as usize)]) {
            Ok(v) => return Err(v.into()),
            Err(e) => return Err(format!("parse error msg failed, {}", e).into()),
        };
    }

    Ok(())
}

/// Gets the size of the attestation report that would be generated using the given parameters.
///
/// # Arguments
///
/// * `params_json_str` - The JSON string representation of the parameters.
///
/// # Returns
///
/// A result containing either the size of the attestation report or an error message.
pub fn get_attestation_report_size(params_json_str: &str) -> Result<u32, Error> {
    let c_params_json_str = CString::new(params_json_str.as_bytes())
        .map_err(|_| Error::from("attestation_report_verify: params_json_str ffi failure"))?;
    let mut report_len: u32 = 0;
    let code = unsafe {
        trustflow_attestation_sys::GetAttestationReportSize(
            c_params_json_str.as_ptr(),
            params_json_str.len() as u32,
            &mut report_len,
        )
    };
    if code != 0 {
        return Err(format!("get_attestation_report_size failed").into());
    }
    return Ok(report_len);
}

/// Generates an attestation report using the given parameters.
///
/// # Arguments
///
/// * `params_json_str` - The JSON string representation of the parameters.
///
/// # Returns
///
/// A result containing either the generated attestation report or an error message.
pub fn generate_attestation_report(params_json_str: &str) -> Result<std::string::String, Error> {
    let mut report_len = get_attestation_report_size(params_json_str)?;
    let mut report_buf = vec![0; report_len as usize];

    let mut msg_buf = vec![0; BUFFER_SIZE_IN_BYTES as usize];
    let mut msg_len: u32 = BUFFER_SIZE_IN_BYTES;
    let mut details_len: u32 = 0;

    let c_params_json_str = CString::new(params_json_str.as_bytes())
        .map_err(|_| Error::from("attestation_report_verify: params_json_str ffi failure"))?;

    let code = unsafe {
        trustflow_attestation_sys::GenerateAttestationReport(
            c_params_json_str.as_ptr(),
            params_json_str.len() as u32,
            report_buf.as_mut_ptr() as *mut i8,
            &mut report_len,
            msg_buf.as_mut_ptr() as *mut i8,
            &mut msg_len,
            std::ptr::null_mut(),
            &mut details_len,
        )
    };

    if code != 0 {
        match std::str::from_utf8(&msg_buf[..(msg_len as usize)]) {
            Ok(v) => return Err(v.into()),
            Err(e) => return Err(format!("parse error msg failed, {}", e).into()),
        };
    }

    return Ok(std::str::from_utf8(&report_buf[..(report_len as usize)])?.to_string());
}


