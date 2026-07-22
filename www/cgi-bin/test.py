#!/usr/bin/python3
import os, sys

# 1. On ajoute "charset=utf-8" pour les accents
print("Content-Type: text/html; charset=utf-8\r\n\r\n")

print("""
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <style>
        body { font-family: 'Segoe UI', sans-serif; background-color: #f4f7f6; padding: 40px; color: #333; }
        .result-card { 
            background: white; 
            padding: 30px; 
            border-radius: 12px; 
            box-shadow: 0 4px 15px rgba(0,0,0,0.1);
            max-width: 600px;
            margin: 0 auto;
            border-top: 5px solid #2980b9;
        }
        h1 { color: #2980b9; margin-top: 0; }
        .data-box { 
            background: #f8f9fa; 
            padding: 15px; 
            border-left: 4px solid #2ecc71; 
            margin: 20px 0;
            font-family: monospace;
            word-break: break-all;
        }
        .back-btn {
            display: inline-block;
            text-decoration: none;
            background: #2980b9;
            color: white;
            padding: 10px 20px;
            border-radius: 5px;
            transition: background 0.3s;
        }
        .back-btn:hover { background: #3498db; }
    </style>
</head>
<body>
    <div class="result-card">
        <h1>🚀 Succès du CGI</h1>
""")

method = os.environ.get('REQUEST_METHOD', 'UNKNOWN')
print(f"<p><b>Méthode utilisée :</b> <span style='color:#e67e22'>{method}</span></p>")

if method == "POST":
    body = sys.stdin.read()
    print("<p><b>Données brutes reçues du client :</b></p>")
    print(f"<div class='data-box'>{body}</div>")

print("""
        <hr>
        <a href="/index.html" class="back-btn">⬅️ Retour au Panel</a>
    </div>
</body>
</html>
""")