import pygame
import sys
from gamestate import ChessBoard, WHITE, BLACK, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
from graphics.pieces import *
from graphics.button import Button
from graphics.animator import PieceAnimation

pygame.init()

MONITOR_INFO = pygame.display.Info()
WINDOW_WIDTH = MONITOR_INFO.current_w
WINDOW_HEIGHT = MONITOR_INFO.current_h
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.NOFRAME)
pygame.display.set_caption("PyChess")

TILE_SIZE = min(WINDOW_WIDTH, WINDOW_HEIGHT) // 10


LEFT_SIDE_BUFFER = 50 # board buffer from left side of screen

BOARD_OFFSET_X = LEFT_SIDE_BUFFER
BOARD_OFFSET_Y = (WINDOW_HEIGHT - (TILE_SIZE * 8)) // 2

# --- CAPTURE BARS CONFIGURATION ---
# Placing the sidebar right after the board ends
CAPTURE_BAR_WIDTH = TILE_SIZE * 6
CAPTURE_BAR_HEIGHT = TILE_SIZE/1.5
CAPTURE_BAR_X = BOARD_OFFSET_X #+ (TILE_SIZE*8-CAPTURE_BAR_WIDTH)/2

# White captures go near the bottom, Black captures go near the top
WHITE_CAPTURE_BAR_Y = WINDOW_HEIGHT - CAPTURE_BAR_HEIGHT/2 - BOARD_OFFSET_Y/2 
BLACK_CAPTURE_BAR_Y = -CAPTURE_BAR_HEIGHT/2 + BOARD_OFFSET_Y/2

PROMOTION_MENU_WIDTH = 400
PROMOTION_MENU_HEIGHT = 100
PROMOTION_MENU_X = (WINDOW_WIDTH - PROMOTION_MENU_WIDTH) // 2
PROMOTION_MENU_Y = (WINDOW_HEIGHT - PROMOTION_MENU_HEIGHT) // 2

GAME_OVER_WIDTH = 500
GAME_OVER_HEIGHT = 150
GAME_OVER_X = (WINDOW_WIDTH - GAME_OVER_WIDTH) // 2
GAME_OVER_Y = (WINDOW_HEIGHT - GAME_OVER_HEIGHT) // 2


ACTION_BTN_W = TILE_SIZE/1.2
ACTION_BTN_H = TILE_SIZE/1.8


RESIGN_BTN_X = CAPTURE_BAR_X + CAPTURE_BAR_WIDTH + (TILE_SIZE-ACTION_BTN_W)/2
DRAW_BTN_X = CAPTURE_BAR_X + CAPTURE_BAR_WIDTH + TILE_SIZE +(TILE_SIZE-ACTION_BTN_W)/2
ACTION_BTN_Y_WHITE = WHITE_CAPTURE_BAR_Y + (CAPTURE_BAR_HEIGHT - ACTION_BTN_H) / 2
ACTION_BTN_Y_BLACK = BLACK_CAPTURE_BAR_Y + (CAPTURE_BAR_HEIGHT - ACTION_BTN_H) / 2

LIGHT_SQUARE_COLOR = (234, 235, 239) #yk
DARK_SQUARE_COLOR = (134, 142, 151) #yk
SELECTED_HIGHLIGHT_COLOR = (159, 204, 239) #highlight for the square u selected
LAST_MOVE_HIGHLIGHT_COLOR = (159, 204, 239)
PREMOVE_HIGHLIGHT_COLOR = (176, 198, 232) #dot color for legal moves
CHECK_INDICATOR_COLOR = (240, 90, 90)


WHITE_PIECE_COLOR = (255, 255, 255)
WHITE_PIECE_OUTLINE = (0, 0, 0)
BLACK_PIECE_COLOR = (50, 50, 50)
BLACK_PIECE_OUTLINE = (200, 200, 200)


def draw_chessboard(screen_surface, selected_square, last_move, game_board, in_animation):


    white_in_check = game_board.is_in_check(WHITE)
    black_in_check = game_board.is_in_check(BLACK)
    
    white_king_pos = game_board.find_king_position(WHITE)
    black_king_pos = game_board.find_king_position(BLACK)


    screen_surface.fill((0, 0, 0)) 
    
    for row in range(8):
        for column in range(8):
            square_color = LIGHT_SQUARE_COLOR if (row + column) % 2 == 0 else DARK_SQUARE_COLOR

            if last_move is not None:
                start_pos, end_pos = last_move
                if (row, column) == start_pos or (row, column) == end_pos:
                    square_color = LAST_MOVE_HIGHLIGHT_COLOR
            if not in_animation and hasattr(game_board, 'king_in_check') and game_board.king_in_check:
                if white_in_check and (row, column) == white_king_pos:
                    square_color = CHECK_INDICATOR_COLOR
                if black_in_check and (row, column) == black_king_pos:
                    square_color = CHECK_INDICATOR_COLOR
            
            if selected_square == (row, column):
                square_color = SELECTED_HIGHLIGHT_COLOR
                
            square_x = BOARD_OFFSET_X + (column * TILE_SIZE)
            square_y = BOARD_OFFSET_Y + (row * TILE_SIZE)
            pygame.draw.rect(screen_surface, square_color, (square_x, square_y, TILE_SIZE, TILE_SIZE))

def draw_pieces(screen_surface, grid):
    mouse_x, mouse_y = pygame.mouse.get_pos()
    
    for row in range(8):
        for col in range(8):
            piece_object = grid[row][col]
            
            if piece_object is not None:
                tile_left = BOARD_OFFSET_X + (col * TILE_SIZE)
                tile_top = BOARD_OFFSET_Y + (row * TILE_SIZE)
                
                center_x = tile_left + (TILE_SIZE // 2)
                center_y = tile_top + (TILE_SIZE // 2)
                radius = int(TILE_SIZE * 0.45)
                
                if tile_left <= mouse_x < tile_left + TILE_SIZE and tile_top <= mouse_y < tile_top + TILE_SIZE:
                    tilt_angle = -8  
                else:
                    tilt_angle = 0   
                if "white" in str(piece_object.color).lower() or piece_object.color == WHITE:
                    fill = (248, 248, 248)
                    outline = (45, 45, 45)
                else:
                    fill = (86, 83, 82)
                    outline = (25, 25, 25)
                
                draw_piece(piece_object.type, screen_surface, fill, outline, center_x, center_y, radius, angle=tilt_angle)

def draw_legal_moves(screen_surface, legal_moves, game_grid):
    for row, column in legal_moves: 
        center_x = BOARD_OFFSET_X + (column * TILE_SIZE) + (TILE_SIZE // 2)
        center_y = BOARD_OFFSET_Y + (row * TILE_SIZE) + (TILE_SIZE // 2)
        
        
        target_piece = game_grid[row][column]
        
        if target_piece is not None:
        
            ring_radius = TILE_SIZE // 3 + 5
            pygame.draw.circle(screen_surface, PREMOVE_HIGHLIGHT_COLOR, (center_x, center_y), ring_radius, 8)
        else:
            
            pygame.draw.circle(screen_surface, PREMOVE_HIGHLIGHT_COLOR, (center_x, center_y), TILE_SIZE // 7)

def draw_game_over_screen(screen_surface, losing_color, game_over_buttons, game_board):
    
    overlay = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
    overlay.fill((0, 0, 0, 180))
    screen_surface.blit(overlay, (0, 0))

    
    pygame.draw.rect(screen_surface, (40, 40, 40), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT),0,0,15,15,15,15)
    pygame.draw.rect(screen_surface, (230, 50, 50), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT),4,15)

    
    if game_board.game_over_reason == "WHITE_RESIGN":
        end_text = "BLACK WINS! (White Resigned)"
    elif game_board.game_over_reason == "BLACK_RESIGN":
        end_text = "WHITE WINS! (Black Resigned)"
    elif game_board.game_over_reason == "DRAW":
        end_text = "GAME DRAWN BY AGREEMENT"
    elif game_board.game_over_reason == "FIFTY_MOVE_RULE":
        end_text = "DRAW! (50-Move Rule Reached)"
    elif game_board.game_over_reason == "Stalemate":
        end_text = "DRAW! (Stalemate)"
    elif game_board.game_over_reason == "Repetition":
        end_text = "DRAW! (Threefold Repetition)"
    elif game_board.game_over_reason == "Checkmate":
        end_text = "BLACK WINS BY MATE!" if losing_color == WHITE else "WHITE WINS BY MATE!"
    else:
        end_text = "GAME OVER (ERROR)"
    font_title = pygame.font.SysFont("arial", 40, bold=True)

    title_surface = font_title.render(end_text, True, (255, 255, 255))

    
    screen_surface.blit(title_surface, (GAME_OVER_X + (GAME_OVER_WIDTH - title_surface.get_width()) // 2, GAME_OVER_Y + 30))

    for button in game_over_buttons:
        button.draw(screen_surface)
        
def draw_captured_bars(screen_surface, captured_white, captured_black):
    PANEL_COLOR = (30, 30, 30)
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, WHITE_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT),0,0,0,0,5,5)
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, BLACK_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT),0,0,5,5)

    
    def draw_mini_piece(piece, x, y): 
        radius = TILE_SIZE // 4
        fill_color = WHITE_PIECE_COLOR if piece.color == WHITE else BLACK_PIECE_COLOR
        outline_color = WHITE_PIECE_OUTLINE if piece.color == WHITE else BLACK_PIECE_OUTLINE
        
        draw_piece(piece.type, screen_surface, fill_color, outline_color, x, y, radius)

    
    for index, piece in enumerate(captured_white):
        column_offset = (index % 8) * (TILE_SIZE // 2)
        draw_mini_piece(piece, CAPTURE_BAR_X + 30 + column_offset, BLACK_CAPTURE_BAR_Y+CAPTURE_BAR_HEIGHT/2-CAPTURE_BAR_HEIGHT/15)

    
    for index, piece in enumerate(captured_black):
        column_offset = (index % 8) * (TILE_SIZE // 2)
        draw_mini_piece(piece, CAPTURE_BAR_X + 30 + column_offset, WHITE_CAPTURE_BAR_Y+CAPTURE_BAR_HEIGHT/2-CAPTURE_BAR_HEIGHT/15)


def draw_promotion_menu(screen_surface, turn_color):
    
    overlay = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
    overlay.fill((0, 0, 0, 150))
    screen_surface.blit(overlay, (0, 0))

    
    pygame.draw.rect(screen_surface, (50, 50, 50), (PROMOTION_MENU_X, PROMOTION_MENU_Y, PROMOTION_MENU_WIDTH, PROMOTION_MENU_HEIGHT))
    pygame.draw.rect(screen_surface, (200, 200, 200), (PROMOTION_MENU_X, PROMOTION_MENU_Y, PROMOTION_MENU_WIDTH, PROMOTION_MENU_HEIGHT), 3)


    
    button_width = PROMOTION_MENU_WIDTH // 4
    piece_types = [QUEEN, ROOK, BISHOP, KNIGHT]
    
    fill_color = WHITE_PIECE_COLOR if turn_color == WHITE else BLACK_PIECE_COLOR
    outline_color = WHITE_PIECE_OUTLINE if turn_color == WHITE else BLACK_PIECE_OUTLINE

    for index, piece_type in enumerate(piece_types):
        btn_x = PROMOTION_MENU_X + (index * button_width) + (button_width // 2)
        btn_y = PROMOTION_MENU_Y + (PROMOTION_MENU_HEIGHT // 2)
        
        draw_piece(piece_type, screen_surface, fill_color, outline_color, btn_x, btn_y, TILE_SIZE // 3)

def main():
    game_board = ChessBoard()
    selected_square = None
    is_game_running = True
    board_offset = (BOARD_OFFSET_X, BOARD_OFFSET_Y)

    center_panel_y = (WINDOW_HEIGHT - 220) // 2
    btn_w, btn_h = 160, 50
    btn_left_x = (WINDOW_WIDTH // 2) - btn_w - 20
    btn_right_x = (WINDOW_WIDTH // 2) + 20
    btn_y = center_panel_y + 120

    rematch_btn = Button(btn_left_x, btn_y, btn_w, btn_h, "Rematch", (50, 150, 50), (70, 190, 70))
    new_game_btn = Button(btn_right_x, btn_y, btn_w, btn_h, "New Game", (70, 70, 180), (100, 100, 230))
    game_buttons = [rematch_btn, new_game_btn]
    resign_btn = Button(RESIGN_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Resign", (180, 60, 60), (120, 40, 40), font_size=16)
    draw_btn = Button(DRAW_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Draw", (140, 140, 140), (95, 95, 95), font_size=16)
        
    active_animation = None
    hidden_piece_data = None
    while is_game_running:
        mouse_pos = pygame.mouse.get_pos()
        if active_animation and active_animation.is_active:
            game_is_over = False
        else:
            game_is_over = game_board.game_over()
        if game_is_over:
            rematch_btn.is_hover(mouse_pos)
            new_game_btn.is_hover(mouse_pos)
        else:
            resign_btn.is_hover(mouse_pos)
            draw_btn.is_hover(mouse_pos)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                is_game_running = False
                
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    is_game_running = False
            
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if active_animation and active_animation.is_active:
                    continue
            
            
                mouse_x, mouse_y = pygame.mouse.get_pos()
                if game_board.promotion_required:
                    
                    if PROMOTION_MENU_Y <= mouse_y <= PROMOTION_MENU_Y + PROMOTION_MENU_HEIGHT:
                        if PROMOTION_MENU_X <= mouse_x <= PROMOTION_MENU_X + PROMOTION_MENU_WIDTH:
                            relative_x = mouse_x - PROMOTION_MENU_X
                            button_width = PROMOTION_MENU_WIDTH // 4
                            clicked_button_index = relative_x // button_width
                            
                            piece_options = [QUEEN, ROOK, BISHOP, KNIGHT]
                            chosen_upgrade = piece_options[clicked_button_index]
                            
                            game_board.promote_pawn(chosen_upgrade)
                    continue 
                
                if game_is_over:
                    if rematch_btn.is_clicked(mouse_pos, event.type):
                        game_board = ChessBoard()
                        selected_square = None
                        continue
                    elif new_game_btn.is_clicked(mouse_pos, event.type):
                        game_board = ChessBoard()
                        selected_square = None
                        continue
                
                
                if not game_is_over:
                    if resign_btn.is_clicked(mouse_pos, event.type):
                        game_board.resign(game_board.turn)
                        selected_square = None
                        continue
                    elif draw_btn.is_clicked(mouse_pos, event.type):
                        game_board.declare_draw()
                        selected_square = None
                        continue

                clicked_column = (mouse_x - BOARD_OFFSET_X) // TILE_SIZE
                clicked_row = (mouse_y - BOARD_OFFSET_Y) // TILE_SIZE

                
                if 0 <= clicked_row < 8 and 0 <= clicked_column < 8:
                    
                    if selected_square is None:
                        clicked_piece = game_board.grid[clicked_row][clicked_column]
                        if clicked_piece and clicked_piece.color == game_board.turn:
                            selected_square = (clicked_row, clicked_column)
                    
                    
                    else:
                        start_row, start_column = selected_square
                        
                        
                        available_legal_moves = game_board.get_safe_legal_moves(start_row, start_column)
                        
                        
                        if (clicked_row, clicked_column) in available_legal_moves:
                            moving_piece = game_board.grid[start_row][start_column]
                            
                            active_animation = PieceAnimation(
                                moving_piece.type,
                                moving_piece.color,
                                (start_row, start_column),
                                (clicked_row, clicked_column),
                                TILE_SIZE,
                                board_offset,
                                speed=0.2
                            )
                            
                            hidden_piece_data = {
                                "row": clicked_row,
                                "col": clicked_column,
                                "piece": moving_piece
                            }
                            
                            is_pawn = moving_piece.type == 1
                            is_diagonal = start_column != clicked_column
                            is_target_empty = game_board.grid[clicked_row][clicked_column] is None
                            
                            if is_pawn and is_diagonal and is_target_empty:
                                game_board.grid[start_row][clicked_column] = None
                            # ------------------------------

                            game_board.make_move(selected_square, (clicked_row, clicked_column))
                            
                            game_board.grid[clicked_row][clicked_column] = None
                            
                        selected_square = None
                        
                else:
                    
                    selected_square = None
                if game_board.turn == WHITE:
                    resign_btn = Button(RESIGN_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Resign", (180, 60, 60), (120, 40, 40), font_size=16)
                    draw_btn = Button(DRAW_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Draw", (140, 140, 140), (95, 95, 95), font_size=16)
                else:
                    resign_btn = Button(RESIGN_BTN_X, ACTION_BTN_Y_BLACK, ACTION_BTN_W, ACTION_BTN_H, "Resign", (180, 60, 60), (120, 40, 40), font_size=16)
                    draw_btn = Button(DRAW_BTN_X, ACTION_BTN_Y_BLACK, ACTION_BTN_W, ACTION_BTN_H, "Draw", (140, 140, 140), (95, 95, 95), font_size=16)

        in_animation = active_animation is not None and active_animation.is_active
        draw_chessboard(screen, selected_square, game_board.last_move, game_board, in_animation)

        if selected_square is not None:
            start_row, start_column = selected_square
            active_legal_moves = game_board.get_safe_legal_moves(start_row, start_column)
            draw_legal_moves(screen, active_legal_moves, game_board.grid)

        draw_pieces(screen, game_board.grid)
        
        if active_animation and active_animation.is_active:
            active_animation.update()
            
            if active_animation.color == WHITE:
                fill, outline = (248, 248, 248), (45, 45, 45)
            else:
                fill, outline = (86, 83, 82), (25, 25, 25)
                
            radius = int(TILE_SIZE * 0.45)
            
            draw_piece(
                active_animation.piece_type,
                screen,
                fill,
                outline,
                int(active_animation.current_x),
                int(active_animation.current_y),
                radius,
                angle=0 
            )
        elif hidden_piece_data:
            game_board.grid[hidden_piece_data["row"]][hidden_piece_data["col"]] = hidden_piece_data["piece"]
            hidden_piece_data = None
            active_animation = None
            
        draw_captured_bars(screen, game_board.captured_white, game_board.captured_black)
        if not game_is_over:
            resign_btn.draw(screen)
            draw_btn.draw(screen)
        
        if game_board.promotion_required:
            draw_promotion_menu(screen, game_board.turn)

        if game_is_over:
            draw_game_over_screen(screen, game_board.turn, game_buttons, game_board)
            
        pygame.display.flip()

        
    pygame.quit()
    sys.exit()

def mainMenu():
    main()

if __name__ == "__main__":
    mainMenu()


