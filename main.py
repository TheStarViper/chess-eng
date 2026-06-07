import pygame
import sys
from gamestate import ChessBoard, WHITE, BLACK, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
from graphics.pieces import *


pygame.init()

MONITOR_INFO = pygame.display.Info()
WINDOW_WIDTH = MONITOR_INFO.current_w
WINDOW_HEIGHT = MONITOR_INFO.current_h
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.FULLSCREEN)
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


def draw_chessboard(screen_surface, selected_square, last_move, game_board):


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

            if white_in_check and (row, column) == white_king_pos:
                square_color = CHECK_INDICATOR_COLOR
            if black_in_check and (row, column) == black_king_pos:
                square_color = CHECK_INDICATOR_COLOR
            
            if selected_square == (row, column):
                square_color = SELECTED_HIGHLIGHT_COLOR
                
            square_x = BOARD_OFFSET_X + (column * TILE_SIZE)
            square_y = BOARD_OFFSET_Y + (row * TILE_SIZE)
            pygame.draw.rect(screen_surface, square_color, (square_x, square_y, TILE_SIZE, TILE_SIZE))

def draw_pieces(screen_surface, game_grid):
    for row in range(8):
        for column in range(8):
            piece = game_grid[row][column]
            if piece is None:
                continue

            center_x = BOARD_OFFSET_X + (column * TILE_SIZE) + (TILE_SIZE // 2)
            center_y = BOARD_OFFSET_Y + (row * TILE_SIZE) + (TILE_SIZE // 2)
            radius = TILE_SIZE // 3

            fill_color = WHITE_PIECE_COLOR if piece.color == WHITE else BLACK_PIECE_COLOR
            outline_color = WHITE_PIECE_OUTLINE if piece.color == WHITE else BLACK_PIECE_OUTLINE

            draw_piece(piece.type, screen_surface, fill_color, outline_color, center_x, center_y, radius)

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

def draw_captured_bars(screen_surface, captured_white, captured_black):
    PANEL_COLOR = (30, 30, 30)
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, WHITE_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT))
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, BLACK_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT))

    
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
    # Translucent darkening box over everything
    overlay = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
    overlay.fill((0, 0, 0, 150))
    screen_surface.blit(overlay, (0, 0))

    # Menu Panel Background Box
    pygame.draw.rect(screen_surface, (50, 50, 50), (PROMOTION_MENU_X, PROMOTION_MENU_Y, PROMOTION_MENU_WIDTH, PROMOTION_MENU_HEIGHT))
    pygame.draw.rect(screen_surface, (200, 200, 200), (PROMOTION_MENU_X, PROMOTION_MENU_Y, PROMOTION_MENU_WIDTH, PROMOTION_MENU_HEIGHT), 3)


    # Define 4 click target boxes for Queen, Rook, Bishop, Knight
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

    while is_game_running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                is_game_running = False
                
            elif event.type == pygame.MOUSEBUTTONDOWN:
                mouse_x, mouse_y = pygame.mouse.get_pos()
                
                if game_board.promotion_required:
                    # Check if click lands within menu vertical bounds
                    if PROMOTION_MENU_Y <= mouse_y <= PROMOTION_MENU_Y + PROMOTION_MENU_HEIGHT:
                        if PROMOTION_MENU_X <= mouse_x <= PROMOTION_MENU_X + PROMOTION_MENU_WIDTH:
                            relative_x = mouse_x - PROMOTION_MENU_X
                            button_width = PROMOTION_MENU_WIDTH // 4
                            clicked_button_index = relative_x // button_width
                            
                            piece_options = [QUEEN, ROOK, BISHOP, KNIGHT]
                            chosen_upgrade = piece_options[clicked_button_index]
                            
                            game_board.promote_pawn(chosen_upgrade)
                    continue # Bypass normal board evaluation processing

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
                            game_board.make_move(selected_square, (clicked_row, clicked_column))
                        selected_square = None
                        
                else:
                    
                    selected_square = None

        
        draw_chessboard(screen, selected_square, game_board.last_move, game_board)
        draw_pieces(screen, game_board.grid)
        draw_captured_bars(screen, game_board.captured_white, game_board.captured_black)
        
        if selected_square is not None:
            start_row, start_column = selected_square
            active_legal_moves = game_board.get_safe_legal_moves(start_row, start_column)
            draw_legal_moves(screen, active_legal_moves, game_board.grid)
        
        if game_board.promotion_required:
            draw_promotion_menu(screen, game_board.turn)
        pygame.display.flip()

    pygame.quit()
    sys.exit()

def mainMenu():
    main()

if __name__ == "__main__":
    mainMenu()


