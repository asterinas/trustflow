:notoc:

TrustFlow
=============

`TrustFlow <https://github.com/asterinas/trustflow>`_ 是隐语基于可信硬件的隐私保护引擎。
TrustFlow立足于可信执行环境技术，提供受保护和隔离的环境，其中封装了敏感数据，并且提供数据安全存储和计算能力。


可信执行环境(Trusted Execution Environment，TEE) 是一种基于硬件的隐私保护技术。它保证了执行代码的真实性，运行时状态(如寄存器、内存和敏感I/O)的完整性，
以及存储在内存中的代码、数据和运行时状态的机密性。此外，还能够向第三方提供远程认证，以证明其可靠性。基于TEE，TrustFlow可以保护数据的机密性、完整性和可用性。

在TrustFlow中，数据被加密并存储在受控和限制的环境，以防止未经授权访问。
TrustFlow采用多种安全措施，如远程认证、计算隔离、授权管控和审计机制，以确保数据被正确保护。
TrustFlow具有端到端全链路加密能力，提供了零信任安全特性，机构可以对TrustFlow进行验证，限制数据使用最小权限访问。
与此同时，TrustFlow提供了丰富的数据加工处理能力，包括数据预处理、经典机器学习、深度学习、大模型、数据分析等，
在保护数据隐私的同时可以充分释放数据价值。

TrustFlow可用于需要安全存储、处理或共享敏感数据的场景，以最大程度地减少暴露或未经授权使用的风险。

.. image:: images/trustflow.jpg

为什么选择TrustFlow
---------------------

TrustFlow允许机构在保护数据安全的前提下，探索强大的人工智能（AI）以及大数据分析（BI）技术，充分释放数据价值。
TrustFlow保护了数据使用中（data-in-use）、数据存储（data-at-rest）、数据传输（data-in-transit）的安全。
使用TrustFlow，你可以获得丰富的可信应用。

- **可信传统机器学习**：基于TrustFlow的传统机器学习能力，用户可以以安全可信的方式运行诸如逻辑回归、树模型等机器学习。

- **可信深度学习**：基于TrustFlow的深度学习能力，用户可以以安全可信的方式运行常用的深度学习框架（比如PyTorch、TensorFlow）。

- **可信大模型**：基于TrustFlow的大模型能力，用户可以以安全可信的方式对大模型进行微调（fine-tune）以及部署大模型在线服务。

- **可信分析**：基于TrustFlow的数据分析能力，用户可以以安全可信的方式对数据进行诸如求交、聚合、统计等分析。

入门
-----
从零开始，体验如何使用TrustFlow进行联合建模，欢迎阅读 `快速上手 <quick_start/index>`_ 。

架构和设计
-----------
了解TrustFlow架构、原理和更多功能，欢迎阅读 `架构设计 <architecture/index>`_ 。

1. `TrustFlow核心原理 <architecture/principle>`_
2. `授权策略 <architecture/policy>`_
3. `可信应用 <architecture/apps/index>`_

自定义可信组件
---------------
开发新的自定义可信组件，欢迎阅读 `新组件开发教程 <development/new_component>`_ 。

高阶话题
-----------
关于TrustFlow的更多话题，欢迎阅读 `高阶话题 <advanced_topic/index>`_ 。

1. `基于TrustFlow的跨域管控 <advanced_topic/cross_domain_controll_in_tee>`_


路线图
-----------
TrustFlow功能列表和路线图，欢迎阅读 `路线图 <./advanced_topic/roadmap>`_。

获得帮助
------------
使用 TrustFlow 时遇到问题？在这里找到获得帮助的方式。

- TrustFlow 的 `Issues <https://github.com/asterinas/trustflow/issues>`_ 

.. toctree::
   :maxdepth: 2
   :hidden:
   :titlesonly:

   quick_start/index
   architecture/index
   development/index
   advanced_topic/index
