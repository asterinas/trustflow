import Verifier from './trustflow_verifier';
import { Input, Button, Form } from 'antd';

export default function HomePage() {

  const policy = '';

  const [form] = Form.useForm();

  const verify = async (report: string, policy: string) => {
    const wasmModule = await Verifier();
    console.log(wasmModule);
    const res = await wasmModule.ready.then(() => {
      const status = wasmModule.evidenceVerify(report, policy);
      return status;
    });

    return res;
  };

  return (
    <div>
      <h2>Yay! Welcome to WASM RA Test (react sample app)!</h2>
      <Form layout={'vertical'} form={form} onFinish={async (val)=>{
        const {raReport, policy} = val
        const res = await verify(raReport, policy);
          alert(JSON.stringify(res));
      }}>
        <Form.Item name={'policy'} label={'policy'} initialValue={policy}  rules={[{ required: true, message: 'Please input policy' }]}>
          <Input></Input>
          </Form.Item>

        <Form.Item name={'raReport'} label={'report'}  rules={[{ required: true, message: 'Please input report' }]}>
        <Input.TextArea  autoSize={{ minRows: 4, maxRows: 8 }} />

        </Form.Item>
        <Button
        style={{ marginTop: 10 }}
        type={'primary'}
        htmlType='submit'
      >
          verify
      </Button>

      </Form>
    </div>
  );
}
