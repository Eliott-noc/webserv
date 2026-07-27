import requests
import unittest

# Update this to match your webserver's local address and port
BASE_URL = "http://127.0.0.1:8080"

class TestWebServer(unittest.TestCase):

    def test_homepage_status(self):
        """Test if the homepage is up and returns a 200 OK status."""
        try:
            response = requests.get(f"{BASE_URL}/")
            self.assertEqual(response.status_code, 200, "Homepage did not return 200 OK")
        except requests.exceptions.ConnectionError:
            self.fail("Connection refused. Is the webserver running?")

    def test_404_not_found(self):
        """Test how the server handles routes that don't exist."""
        response = requests.get(f"{BASE_URL}/this-route-is-fake")
        self.assertEqual(response.status_code, 404, "Server did not return a 404 for a missing page")

    def test_post_request(self):
        """Example: Testing a POST request (modify endpoint as needed)."""
        # Change '/api/data' to a valid POST endpoint on your server
        endpoint = f"{BASE_URL}/api/data"
        payload = {"user": "test_user", "action": "ping"}
        
        # Uncomment the lines below to test an actual POST route
        # response = requests.post(endpoint, json=payload)
        # self.assertIn(response.status_code, [200, 201])
        pass 

if __name__ == '__main__':
    # verbosity=2 gives you a nice detailed output in the console
    unittest.main(verbosity=2)