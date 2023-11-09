FROM secretflow/anolis8-python:3.8.15 as python
FROM secretflow/trustflow-ci-sim:0.1.0 as builder


RUN apt install protobuf-compiler build-essential -y && apt clean
COPY componentize /home/admin/dev/componentize
COPY setup.py /home/admin/dev/setup.py
COPY MANIFEST.in /home/admin/dev/MANIFEST.in
COPY requirements.txt /home/admin/dev/requirements.txt

WORKDIR /home/admin/dev
RUN export PATH=/root/.cargo/bin:$PATH && python setup.py sdist bdist_wheel

FROM ubuntu:20.04

COPY --from=python /root/miniconda3/envs/secretflow/bin/ /usr/local/bin/
COPY --from=python /root/miniconda3/envs/secretflow/lib/ /usr/local/lib/

RUN grep -rl '#!/root/miniconda3/envs/secretflow/bin' /usr/local/bin/ | xargs sed -i -e 's/#!\/root\/miniconda3\/envs\/secretflow/#!\/usr\/local/g'

ARG config_templates=""
LABEL kuscia.secretflow.config-templates=$config_templates

ARG deploy_templates=""
LABEL kuscia.secretflow.deploy-templates=$deploy_templates

COPY --from=builder /home/admin/dev/dist/*.whl /tmp/tee_dm/
RUN pip install $(ls /tmp/tee_dm/*.whl) && pip cache purge


