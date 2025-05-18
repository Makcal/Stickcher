from threading import Lock

from fastapi import Depends, FastAPI, Request
from fastapi.responses import PlainTextResponse
import easyocr


app = FastAPI()
reader_mutex = Lock()
ocr_reader = easyocr.Reader(['ru', 'en'])


async def get_body(request: Request):
    return await request.body()


@app.post("/scan", response_class=PlainTextResponse)
def scan(body: bytes = Depends(get_body)) -> str:
    with reader_mutex:
        texts: list[str] = ocr_reader.readtext(body, detail=0, paragraph=True) # type: ignore
        result = ' '.join(texts)
        return result


@app.get("/health")
def health_check():
    return {"status": "healthy"}
