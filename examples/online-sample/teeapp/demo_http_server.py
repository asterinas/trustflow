import click
import uvicorn
from fastapi import FastAPI, Request, Response

app = FastAPI()


@app.post("/teeapp")
async def echo_message(request: Request):
    body = await request.body()
    response_content = f"I got your message: {body.decode()}\n"
    return Response(content=response_content, media_type="application/json")


@click.command()
@click.option("--port", type=click.INT, default=8000, help="Port to run the server on")
def run_server(port: int):
    uvicorn.run(app, host="0.0.0.0", port=port)


if __name__ == "__main__":
    run_server()
