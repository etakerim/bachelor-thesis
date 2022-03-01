# gunicorn -b 0.0.0.0:8000 server:app --reload
import falcon

class IoTResource:

    def on_post(self, request, response):
        print(request.headers)
        print(request.bounded_stream.read())


app = falcon.API()
app.add_route('/', IoTResource())
