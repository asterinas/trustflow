可信执行环境简介
========================
可信执行环境（Trusted Execution Environments, TEE）是是一种基于硬件的隐私保护技术。
它保证了执行代码的真实性，运行时状态(如寄存器、内存和敏感I/O)的完整性，
以及存储在内存中的代码、数据和运行时状态的机密性。
此外，还应能够向第三方提供远程认证，以证明其可靠性。

目前包括Intel、AMD、ARM等在内的主流硬件厂商均提供了TEE产品，比如Intel SGX、Intel TDX、
AMD SEV、ARM TrustedZone、ARM CCA、海光CSV等。当前TrustFlow主要支持Intel SGX、
Intel TDX和海光CSV，对其他硬件的支持正在进行中。下列文章对部分TEE的原理进行了介绍，以方便读者了解TEE。


.. toctree::
   :maxdepth: 2

   sgx
   tdx
   csv

