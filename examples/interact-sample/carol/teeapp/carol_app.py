import asyncio

import click
import requests
import uvicorn
from fastapi import FastAPI, Request, Response

app = FastAPI()


@app.post("/teeapp")
async def echo_message(request: Request):
    body = await request.body()
    response_content = f"Hi, I'm Carol. I got your message: {body.decode()}\n"
    return Response(content=response_content, media_type="application/json")


async def call_bob_server():
    while True:
        try:
            response = requests.post(
                "http://trustflow-envoy:41001/teeapp",
                json={"message": "Hello from Carol!"},
            )
            print("response: ", response.text)
        except Exception as e:
            print(f"Call Bob server error: {e}")
        await asyncio.sleep(10)


@app.on_event("startup")
async def startup_event():
    asyncio.create_task(call_bob_server())


@click.command()
@click.option("--port", type=click.INT, default=8000, help="Port to run the server on")
def run_server(port: int):
    uvicorn.run(app, host="0.0.0.0", port=port)


if __name__ == "__main__":
    run_server()
