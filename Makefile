SHELL       = /bin/bash

# ====== CONFIG ======
NAME        = webserv
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -g
RM          = rm -rf

SRC         = $(wildcard *.cpp) $(wildcard src/*.cpp)
OBJ_DIR     = .obj
OBJ         = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

TOTAL_FILES := $(words $(SRC))
COUNT_FILE  := .make_count

# ====== COULEURS & VISUELS ======
DEF_COLOR   = \033[0;39m
GREEN       = \033[1;32m
CYAN        = \033[1;36m
DIM         = \033[2m
YELLOW      = \033[0;33m

.DEFAULT_GOAL := start

# ====== RÈGLES ======

start:
	@clear
	@echo 0 > $(COUNT_FILE)
	@printf "$(DIM)>>> Preparation...$(DEF_COLOR)"
	@frames=("⠋ " "⠙ " "⠹ " "⠸ " "⠼ " "⠴ " "⠦ " "⠧ " "⠇ " "⠏ "); \
	for i in {1..10}; do \
		printf "\r$(DIM)>>> Preparation... $(CYAN)$${frames[$$i]}$(DEF_COLOR)"; \
		sleep 0.04; \
	done
	@printf "\r$(GREEN)[████████████████████] ✓$(DEF_COLOR) Ready to compile\n\n"
	@printf "$(DIM)>>> Compiling $(NAME) (std=c++98)...$(DEF_COLOR)\n    "
	@$(MAKE) --no-print-directory all

all: $(NAME)

$(NAME): $(OBJ)
	@printf " $(GREEN)✓$(DEF_COLOR)\n\n"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@printf "$(GREEN)"
	@echo "    ┌────────────────────────────────────────┐"
	@echo "    │                                        │"
	@echo "    │         [ OK ] C++ Compilation         │"
	@echo "    │                                        │"
	@echo "    │           Executable: $(NAME)          │"
	@echo "    │                                        │"
	@echo "    └────────────────────────────────────────┘"
	@echo ""
	@printf "$(DEF_COLOR)"

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(eval CURRENT_COUNT=$(shell if [ -f $(COUNT_FILE) ]; then cat $(COUNT_FILE); else echo 0; fi))
	@$(eval CURRENT_COUNT=$(shell echo $$(($(CURRENT_COUNT) + 1))))
	@echo $(CURRENT_COUNT) > $(COUNT_FILE)
	@$(eval PERCENT=$(shell echo $$(($(CURRENT_COUNT) * 100 / $(TOTAL_FILES)))))
	@printf "\r$(GREEN)Compiling...[%d/%d] (%d%%)$(DEF_COLOR)" $(CURRENT_COUNT) $(TOTAL_FILES) $(PERCENT)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@printf "$(DIM)>>> Cleaning objects...$(DEF_COLOR)\n"
	@$(RM) $(OBJ_DIR)
	@$(RM) $(COUNT_FILE)
	@printf "    $(GREEN)[████████████████████] ✓$(DEF_COLOR) Done\n"

fclean: clean
	@printf "$(DIM)>>> Removing executable...$(DEF_COLOR)\n"
	@$(RM) $(NAME)
	@printf "    $(GREEN)[████████████████████] ✓$(DEF_COLOR) Done\n\n"
	@printf "$(YELLOW)    FULL CLEAN COMPLETED$(DEF_COLOR)\n"

re: fclean start

.PHONY: start all clean fclean re