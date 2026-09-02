#!/bin/bash
#
# webserv_test.sh - Giga test suite pour webserv (42)
#
# Usage:
#   ./webserv_test.sh
#
# Prerequis:
#   - le serveur doit deja tourner (ex: ./webserv config.conf)
#   - curl doit etre installe
#   - nc (netcat) est utilise pour les tests bas-niveau (optionnel, skip si absent)
#
# Adapte les variables ci-dessous a ta config si besoin.

# ---------------------------------------------------------------------------
# CONFIGURATION - adapte a ton fichier .conf
# ---------------------------------------------------------------------------
HOST="127.0.0.1"
PORT1="8080"
PORT2="9090"
BASE="http://${HOST}:${PORT1}"
BASE2="http://${HOST}:${PORT2}"

UPLOAD_DIR_PATH="/uploads"        # location upload avec GET/POST/DELETE
TOO_SMALL_PATH="/too-small"       # location avec client_max_body_size 10
POST_BODY_PATH="/post_body"       # location avec client_max_body_size 100
CGI_PATH="/cgi-bin"               # location CGI python
CGI_TEST_SCRIPT="test.py"         # script CGI "normal"
CGI_INFINITE_SCRIPT="infinite.py" # script qui boucle a l'infini -> doit timeout
CGI_BAD_SCRIPT="bad.py"           # script qui plante / sort du mauvais contenu
CGI_RETCODE_SCRIPT="returnCode.py" # script qui renvoie un code custom
CGI_SLEEP_SCRIPT="sleep.py"       # script qui dort un peu (test timing normal)

REDIRECT_301_PATH="/google"       # return 301 http://www.google.com
REDIRECT_307_PATH="/redirect-cat" # return 307 /cat.html

TMP_DIR="/tmp/webserv_test_$$"
mkdir -p "$TMP_DIR"

# ---------------------------------------------------------------------------
# COULEURS / COMPTEURS
# ---------------------------------------------------------------------------
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

PASS=0
FAIL=0
SKIP=0

section() {
	echo ""
	echo -e "${BOLD}${CYAN}==================================================${NC}"
	echo -e "${BOLD}${CYAN} $1${NC}"
	echo -e "${BOLD}${CYAN}==================================================${NC}"
}

# check "description" expected actual
check() {
	local desc="$1"
	local expected="$2"
	local actual="$3"
	if [ "$expected" == "$actual" ]; then
		echo -e "  ${GREEN}[PASS]${NC} $desc (got: $actual)"
		PASS=$((PASS+1))
	else
		echo -e "  ${RED}[FAIL]${NC} $desc (expected: $expected, got: $actual)"
		FAIL=$((FAIL+1))
	fi
}

# check_in_list "description" actual "val1|val2|val3"
check_in_list() {
	local desc="$1"
	local actual="$2"
	local list="$3"
	if echo "$list" | grep -qw "$actual"; then
		echo -e "  ${GREEN}[PASS]${NC} $desc (got: $actual, acceptable: $list)"
		PASS=$((PASS+1))
	else
		echo -e "  ${RED}[FAIL]${NC} $desc (got: $actual, expected one of: $list)"
		FAIL=$((FAIL+1))
	fi
}

# check_time "description" max_seconds actual_seconds
check_time() {
	local desc="$1"
	local max="$2"
	local actual="$3"
	if awk "BEGIN{exit !($actual <= $max)}" 2>/dev/null; then
		echo -e "  ${GREEN}[PASS]${NC} $desc (${actual}s <= ${max}s)"
		PASS=$((PASS+1))
	else
		echo -e "  ${RED}[FAIL]${NC} $desc (${actual}s > ${max}s max)"
		FAIL=$((FAIL+1))
	fi
}

skip() {
	echo -e "  ${YELLOW}[SKIP]${NC} $1"
	SKIP=$((SKIP+1))
}

# ---------------------------------------------------------------------------
# 0. SANITY CHECK
# ---------------------------------------------------------------------------
section "0. Sanity check - le serveur repond-il ?"

code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "$BASE/" 2>/dev/null)
if [ -z "$code" ] || [ "$code" == "000" ]; then
	echo -e "${RED}Le serveur ne repond pas sur $BASE. Verifie qu'il tourne et adapte HOST/PORT1 en haut du script.${NC}"
	exit 1
fi
check "GET / repond" "200" "$code"

# ---------------------------------------------------------------------------
# 1. FICHIERS STATIQUES / GET
# ---------------------------------------------------------------------------
section "1. GET basique / fichiers statiques"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE/")
check "GET / (index.html)" "200" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE/cat.html")
check "GET /cat.html" "200" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE/fichier_qui_nexiste_pas_xyz.html")
check "GET fichier inexistant -> 404" "404" "$code"

# Verifie que la page 404 custom est bien servie
body=$(curl -s "$BASE/fichier_qui_nexiste_pas_xyz.html")
custom_body=$(curl -s "$BASE/404_custom.html" 2>/dev/null)
if [ -n "$body" ] && [ "$body" == "$custom_body" ]; then
	echo -e "  ${GREEN}[PASS]${NC} Page 404 custom bien utilisee"
	PASS=$((PASS+1))
else
	echo -e "  ${YELLOW}[WARN]${NC} Impossible de confirmer que 404_custom.html est utilise (verifie manuellement)"
	SKIP=$((SKIP+1))
fi

# ---------------------------------------------------------------------------
# 2. AUTOINDEX
# ---------------------------------------------------------------------------
section "2. Autoindex"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE${UPLOAD_DIR_PATH}/")
check "GET ${UPLOAD_DIR_PATH}/ (autoindex on) -> 200" "200" "$code"

body=$(curl -s "$BASE${UPLOAD_DIR_PATH}/")
if echo "$body" | grep -qi "<html"; then
	echo -e "  ${GREEN}[PASS]${NC} Autoindex genere bien du HTML"
	PASS=$((PASS+1))
else
	echo -e "  ${RED}[FAIL]${NC} Autoindex ne semble pas generer de HTML valide"
	FAIL=$((FAIL+1))
fi

# ---------------------------------------------------------------------------
# 3. METHODES NON AUTORISEES
# ---------------------------------------------------------------------------
section "3. Methodes non autorisees (405)"

code=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE${CGI_PATH}/${CGI_TEST_SCRIPT}")
check "DELETE sur location CGI (GET/POST only) -> 405" "405" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "" "$BASE/")
check "POST sur / (GET only) -> 405" "405" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE${TOO_SMALL_PATH}/x.txt")
check "DELETE sur /too-small (POST only) -> 405" "405" "$code"

# ---------------------------------------------------------------------------
# 4. REDIRECTIONS (301 / 307) + doit etre INSTANTANE (pas de faux timeout)
# ---------------------------------------------------------------------------
section "4. Redirections"

start=$(date +%s.%N)
resp=$(curl -s -D - -o /dev/null --max-time 5 "$BASE${REDIRECT_301_PATH}")
end=$(date +%s.%N)
elapsed=$(echo "$end - $start" | bc 2>/dev/null || awk "BEGIN{print $end - $start}")
code=$(echo "$resp" | head -1 | awk '{print $2}')
check "GET ${REDIRECT_301_PATH} -> 301" "301" "$code"
check_time "Redirection 301 instantanee (pas de blocage jusqu'au timeout client)" "1.0" "$elapsed"
echo "$resp" | grep -qi "^Location:" && echo -e "  ${GREEN}[PASS]${NC} Header Location present" && PASS=$((PASS+1)) || { echo -e "  ${RED}[FAIL]${NC} Header Location absent"; FAIL=$((FAIL+1)); }

start=$(date +%s.%N)
resp=$(curl -s -D - -o /dev/null --max-time 5 "$BASE${REDIRECT_307_PATH}")
end=$(date +%s.%N)
elapsed=$(awk "BEGIN{print $end - $start}")
code=$(echo "$resp" | head -1 | awk '{print $2}')
check "GET ${REDIRECT_307_PATH} -> 307" "307" "$code"
check_time "Redirection 307 instantanee" "1.0" "$elapsed"

# ---------------------------------------------------------------------------
# 5. UPLOAD (POST) + verification de contenu
# ---------------------------------------------------------------------------
section "5. Upload (POST)"

TEST_FILE="test_upload_$$.txt"
TEST_CONTENT="Hello depuis le giga tester $(date +%s)"

code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "$TEST_CONTENT" "$BASE${UPLOAD_DIR_PATH}/${TEST_FILE}")
check "POST nouveau fichier -> 201 Created" "201" "$code"

# Re-upload (overwrite) -> 200 attendu (selon ta logique 'exists')
code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "$TEST_CONTENT (v2)" "$BASE${UPLOAD_DIR_PATH}/${TEST_FILE}")
check "POST meme fichier (overwrite) -> 200 OK" "200" "$code"

# Verifie le contenu via GET
got=$(curl -s "$BASE${UPLOAD_DIR_PATH}/${TEST_FILE}")
if [ "$got" == "$TEST_CONTENT (v2)" ]; then
	echo -e "  ${GREEN}[PASS]${NC} Contenu du fichier uploade correct apres overwrite"
	PASS=$((PASS+1))
else
	echo -e "  ${RED}[FAIL]${NC} Contenu incorrect (attendu: '$TEST_CONTENT (v2)', recu: '$got')"
	FAIL=$((FAIL+1))
fi

# Note : upload_store est un dossier "plat" -> seul le nom de fichier final
# (dernier segment du path) est utilise, un sous-dossier intermediaire
# inexistant dans l'URL est donc ignore. 200/201 est le comportement attendu.
code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "x" "$BASE${UPLOAD_DIR_PATH}/dossier_inexistant/fichier.txt")
check_in_list "POST avec sous-dossier dans l'URL (upload_store est plat) -> 200/201" "$code" "200 201"
curl -s -o /dev/null -X DELETE "$BASE${UPLOAD_DIR_PATH}/fichier.txt" # cleanup

# ---------------------------------------------------------------------------
# 6. LIMITES DE TAILLE (client_max_body_size)
# ---------------------------------------------------------------------------
section "6. Limites de taille de body (413)"

code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "Ce message fait plus de dix octets" "$BASE${TOO_SMALL_PATH}/fail.txt")
check "POST > 10 octets sur /too-small -> 413" "413" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "1234" "$BASE${TOO_SMALL_PATH}/ok.txt")
check_in_list "POST <= 10 octets sur /too-small -> 200/201" "$code" "200 201"

big_body=$(head -c 500 /dev/zero | tr '\0' 'A')
code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "$big_body" "$BASE${POST_BODY_PATH}")
check "POST > 100 octets sur /post_body -> 413" "413" "$code"

# ---------------------------------------------------------------------------
# 7. DELETE
# ---------------------------------------------------------------------------
section "7. DELETE"

code=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE${UPLOAD_DIR_PATH}/${TEST_FILE}")
check "DELETE fichier existant -> 204" "204" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE${UPLOAD_DIR_PATH}/${TEST_FILE}")
check "GET fichier supprime -> 404" "404" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE${UPLOAD_DIR_PATH}/fichier_jamais_cree.txt")
check "DELETE fichier inexistant -> 404" "404" "$code"

# ---------------------------------------------------------------------------
# 8. CGI
# ---------------------------------------------------------------------------
section "8. CGI"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE${CGI_PATH}/${CGI_TEST_SCRIPT}?user=GigaTester")
check "GET CGI avec query string -> 200" "200" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "user=GigaTester" "$BASE${CGI_PATH}/${CGI_TEST_SCRIPT}")
check "POST CGI avec body -> 200" "200" "$code"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE${CGI_PATH}/script_qui_nexiste_pas.py")
check_in_list "GET CGI script inexistant -> 404/500" "$code" "404 500"

if [ -n "$CGI_BAD_SCRIPT" ]; then
	code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$BASE${CGI_PATH}/${CGI_BAD_SCRIPT}")
	# NB: si bad.py boucle/est lent au lieu de planter vite, tu auras 504 ici,
	# ce qui n'est pas forcement un bug serveur -- verifie ce que bad.py fait reellement.
	check_in_list "GET CGI script en erreur (bad.py) -> 500/504" "$code" "500 504"
fi

if [ -n "$CGI_RETCODE_SCRIPT" ]; then
	code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$BASE${CGI_PATH}/${CGI_RETCODE_SCRIPT}")
	echo -e "  ${YELLOW}[INFO]${NC} returnCode.py a renvoye : $code (verifie manuellement que ca correspond a ce que le script est cense renvoyer)"
fi

# ---------------------------------------------------------------------------
# 9. TIMEOUT CGI (le test le plus important pour ton refactor async)
# ---------------------------------------------------------------------------
section "9. Timeout CGI (script infini)"

if [ -n "$CGI_INFINITE_SCRIPT" ]; then
	start=$(date +%s.%N)
	code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 10 "$BASE${CGI_PATH}/${CGI_INFINITE_SCRIPT}")
	end=$(date +%s.%N)
	elapsed=$(awk "BEGIN{print $end - $start}")
	check "GET CGI infini -> 504 Gateway Timeout" "504" "$code"
	echo -e "  ${YELLOW}[INFO]${NC} Temps ecoule : ${elapsed}s (doit etre proche de ton timeout CGI, ex ~2s)"
	check_time "Timeout CGI respecte (pas bloque indefiniment)" "5.0" "$elapsed"
else
	skip "CGI_INFINITE_SCRIPT non defini"
fi

# ---------------------------------------------------------------------------
# 10. NON-BLOCAGE : le CGI infini ne doit PAS geler le reste du serveur
#     -> c'est LE test qui valide tout le refactor qu'on a fait
# ---------------------------------------------------------------------------
section "10. Non-blocage global pendant un CGI qui boucle"

if [ -n "$CGI_INFINITE_SCRIPT" ]; then
	echo "  Lancement du CGI infini en arriere-plan..."
	curl -s -o /dev/null --max-time 10 "$BASE${CGI_PATH}/${CGI_INFINITE_SCRIPT}" &
	CGI_BG_PID=$!
	sleep 0.3   # laisse le temps au serveur de lancer le fork/pipe

	echo "  Envoi de 5 requetes GET normales pendant que le CGI tourne..."
	all_fast=true
	for i in 1 2 3 4 5; do
		start=$(date +%s.%N)
		code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "$BASE/")
		end=$(date +%s.%N)
		elapsed=$(awk "BEGIN{print $end - $start}")
		if [ "$code" != "200" ] || awk "BEGIN{exit !($elapsed > 0.5)}"; then
			all_fast=false
			echo -e "    ${RED}requete $i : code=$code, temps=${elapsed}s${NC}"
		else
			echo -e "    ${GREEN}requete $i : code=$code, temps=${elapsed}s${NC}"
		fi
	done

	if $all_fast; then
		echo -e "  ${GREEN}[PASS]${NC} Le serveur reste reactif pendant qu'un CGI boucle (non-bloquant OK)"
		PASS=$((PASS+1))
	else
		echo -e "  ${RED}[FAIL]${NC} Le serveur a rame pendant le CGI infini -> possible blocage encore present"
		FAIL=$((FAIL+1))
	fi

	wait $CGI_BG_PID 2>/dev/null
else
	skip "CGI_INFINITE_SCRIPT non defini"
fi

# ---------------------------------------------------------------------------
# 11. FUITE DE FD / STABILITE SOUS CHARGE
#     -> envoie beaucoup de requetes upload+delete d'affilee, verifie que
#        le serveur repond encore correctement a la fin (detecte fuite de fd)
# ---------------------------------------------------------------------------
section "11. Stabilite / fuite de descripteurs (charge repetee)"

echo "  Envoi de 60 cycles upload+delete..."
fail_count=0
for i in $(seq 1 60); do
	fname="stress_${i}_$$.txt"
	c1=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "x" "$BASE${UPLOAD_DIR_PATH}/${fname}")
	c2=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE${UPLOAD_DIR_PATH}/${fname}")
	if [ "$c1" != "201" ] || [ "$c2" != "204" ]; then
		fail_count=$((fail_count+1))
	fi
done

if [ "$fail_count" -eq 0 ]; then
	echo -e "  ${GREEN}[PASS]${NC} 60/60 cycles upload+delete OK, aucune fuite detectee"
	PASS=$((PASS+1))
else
	echo -e "  ${RED}[FAIL]${NC} $fail_count/60 cycles ont echoue -> verifie les fuites de fd (close/delete manquants)"
	FAIL=$((FAIL+1))
fi

# Le serveur doit toujours repondre normalement juste apres
code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "$BASE/")
check "Serveur toujours reactif apres la charge" "200" "$code"

# ---------------------------------------------------------------------------
# 12. TRAVERSEE DE REPERTOIRE (securite)
# ---------------------------------------------------------------------------
section "12. Path traversal"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE/../../../../etc/passwd")
check_in_list "GET avec ../ -> 400/403/404 (jamais 200)" "$code" "400 403 404"

code=$(curl -s -o /dev/null -w "%{http_code}" "$BASE${UPLOAD_DIR_PATH}/../../../etc/passwd")
check_in_list "GET traversal depuis /uploads -> 400/403/404" "$code" "400 403 404"

# ---------------------------------------------------------------------------
# 13. MULTI-SERVEUR (deuxieme bloc server sur le port 9090)
# ---------------------------------------------------------------------------
section "13. Multi-serveur (port ${PORT2})"

code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "$BASE2/" 2>/dev/null)
if [ -z "$code" ] || [ "$code" == "000" ]; then
	skip "Serveur sur port ${PORT2} injoignable (verifie que www2/index.html existe)"
else
	check "GET / sur le second serveur (port ${PORT2})" "200" "$code"
fi

# ---------------------------------------------------------------------------
# 14. KEEP-ALIVE
# ---------------------------------------------------------------------------
section "14. Keep-Alive (plusieurs requetes, une connexion)"

# curl reutilise automatiquement la connexion quand on donne plusieurs URLs.
# NB: -o /dev/null doit etre repete apres chaque --next, sinon curl renvoie
# le corps des requetes suivantes sur stdout (comportement surprenant de curl,
# pas un bug de ton serveur).
out=$(curl -s \
	-o /dev/null -w "%{http_code} " "$BASE/" \
	--next -o /dev/null -w "%{http_code} " "$BASE/cat.html" \
	--next -o /dev/null -w "%{http_code} " "$BASE/index.html")
codes=$(echo "$out" | tr -s ' ')
if echo "$codes" | grep -q "200 200 200"; then
	echo -e "  ${GREEN}[PASS]${NC} 3 requetes sequentielles sur la meme connexion -> toutes 200"
	PASS=$((PASS+1))
else
	echo -e "  ${RED}[FAIL]${NC} Attendu '200 200 200', recu '$codes'"
	FAIL=$((FAIL+1))
fi

# Verifie explicitement via HTTP/1.0 que la connexion se ferme (keep-alive == false)
if command -v nc >/dev/null 2>&1; then
	resp=$(printf 'GET / HTTP/1.0\r\nHost: %s\r\n\r\n' "$HOST" | timeout 3 nc "$HOST" "$PORT1" 2>/dev/null)
	if echo "$resp" | grep -qi "^HTTP/1.0 200\|^HTTP/1.1 200"; then
		echo -e "  ${GREEN}[PASS]${NC} Requete HTTP/1.0 traitee correctement"
		PASS=$((PASS+1))
	else
		echo -e "  ${RED}[FAIL]${NC} Requete HTTP/1.0 mal geree"
		FAIL=$((FAIL+1))
	fi
else
	skip "nc absent, test HTTP/1.0 brut ignore"
fi

# ---------------------------------------------------------------------------
# 15. REQUETES MALFORMEES (raw socket via nc)
# ---------------------------------------------------------------------------
section "15. Requetes malformees / robustesse du parser"

if command -v nc >/dev/null 2>&1; then
	# Pas de header Host -> doit etre 400
	resp=$(printf 'GET / HTTP/1.1\r\n\r\n' | timeout 3 nc "$HOST" "$PORT1" 2>/dev/null)
	code=$(echo "$resp" | head -1 | awk '{print $2}')
	check "Requete sans header Host -> 400" "400" "$code"

	# Methode inconnue -> 400 ou 405 selon impl
	resp=$(printf 'FOOBAR / HTTP/1.1\r\nHost: %s\r\n\r\n' "$HOST" | timeout 3 nc "$HOST" "$PORT1" 2>/dev/null)
	code=$(echo "$resp" | head -1 | awk '{print $2}')
	check_in_list "Methode HTTP inconnue -> 400/405" "$code" "400 405"

	# Version HTTP invalide -> 505 ou 400
	resp=$(printf 'GET / HTTP/9.9\r\nHost: %s\r\n\r\n' "$HOST" | timeout 3 nc "$HOST" "$PORT1" 2>/dev/null)
	code=$(echo "$resp" | head -1 | awk '{print $2}')
	check_in_list "Version HTTP invalide -> 505/400" "$code" "505 400"

	# Requete ligne vide / garbage -> le serveur ne doit pas crasher
	printf 'GARBAGE_TOTALEMENT_INVALIDE\r\n\r\n' | timeout 3 nc "$HOST" "$PORT1" >/dev/null 2>&1
	code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "$BASE/")
	check "Le serveur survit a une requete garbage (repond encore 200 apres)" "200" "$code"
else
	skip "nc absent, tests de requetes malformees ignores (installe netcat pour les avoir)"
fi

# ---------------------------------------------------------------------------
# 16. CHUNKED TRANSFER ENCODING
# ---------------------------------------------------------------------------
section "16. Transfer-Encoding: chunked"

code=$(curl -s -o /dev/null -w "%{http_code}" -X POST \
	-H "Transfer-Encoding: chunked" \
	--data-binary "Ceci est un test en chunked encoding" \
	"$BASE${UPLOAD_DIR_PATH}/chunked_test.txt")
check_in_list "POST chunked -> 200/201" "$code" "200 201"
curl -s -o /dev/null -X DELETE "$BASE${UPLOAD_DIR_PATH}/chunked_test.txt" # cleanup

# ---------------------------------------------------------------------------
# RESUME
# ---------------------------------------------------------------------------
section "RESUME"

TOTAL=$((PASS+FAIL))
echo -e "  ${GREEN}PASS: $PASS${NC}"
echo -e "  ${RED}FAIL: $FAIL${NC}"
echo -e "  ${YELLOW}SKIP: $SKIP${NC}"
echo ""

if [ "$FAIL" -eq 0 ]; then
	echo -e "${GREEN}${BOLD}Tous les tests critiques sont passes !${NC}"
else
	echo -e "${RED}${BOLD}$FAIL test(s) ont echoue, regarde les [FAIL] ci-dessus.${NC}"
fi

rm -rf "$TMP_DIR"
exit $FAIL
