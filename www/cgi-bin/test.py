# !/usr/bin/python3
import os, sys

# On définit l'encodage pour les accents
print("Content-Type: text/html; charset=utf-8\r\n\r\n")

# On récupère la chaîne d'arguments (ex: user=william&age=20)
query_string = os.environ.get('QUERY_STRING', '')

# Petite logique pour extraire le nom
user_name = "Inconnu"
if "user=" in query_string:
    # On coupe la string pour avoir ce qui est après "user="
    user_name = query_string.split("user=")[1].split("&")[0]

print(f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <style>
        body {{ font-family: sans-serif; background: #eef; padding: 50px; text-align: center; }}
        .card {{ background: white; padding: 30px; border-radius: 15px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); display: inline-block; }}
        h1 {{ color: #2980b9; }}
        .name {{ color: #e67e22; font-weight: bold; font-size: 1.5em; }}
    </style>
</head>
<body>
    <div class="card">
        <h1>👋 Bonjour <span class="name">{user_name}</span> !</h1>
        <p>Ce message a été généré dynamiquement par un script Python.</p>
        <p>Ton serveur Webserv a réussi à me transmettre ton nom via le protocole CGI.</p>
        <br>
        <a href="/index.html">Retour au Panel</a>
    </div>
</body>
</html>
""")