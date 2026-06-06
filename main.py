import pygame
import sys
from gamestate import ChessBoard, WHITE, BLACK, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
from graphics import *

pygame.init()
WINDOW_WIDTH = 640
WINDOW_HEIGHT = 640
TILE_SIZE = WINDOW_WIDTH // 8
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption("Python Variant Chess")

LIGHT_SQUARE_COLOR = (240, 217, 181)
DARK_SQUARE_COLOR = (181, 136, 99)
SELECTED_HIGHLIGHT_COLOR = (130, 151, 105)

WHITE_PIECE_COLOR = (255, 255, 255)
WHITE_PIECE_OUTLINE = (0, 0, 0)
BLACK_PIECE_COLOR = (50, 50, 50)
BLACK_PIECE_OUTLINE = (200, 200, 200)

def draw_chessboard(screen_surface, selected_square):
    for row in range(8):
        for column in range(8):
            square_color = LIGHT_SQUARE_COLOR if (row + column) % 2 == 0 else DARK_SQUARE_COLOR
            if selected_square == (row, column):
                square_color = SELECTED_HIGHLIGHT_COLOR
                
            square_x = column * TILE_SIZE
            square_y = row * TILE_SIZE
            pygame.draw.rect(screen_surface, square_color, (square_x, square_y, TILE_SIZE, TILE_SIZE))

def draw_pieces(screen_surface, game_grid):
    for row in range(8):
        for column in range(8):
            piece = game_grid[row][column]
            if piece is None:
                continue

            center_x = (column * TILE_SIZE) + (TILE_SIZE // 2)
            center_y = (row * TILE_SIZE) + (TILE_SIZE // 2)
            radius = TILE_SIZE // 3

            fill_color = WHITE_PIECE_COLOR if piece.color == WHITE else BLACK_PIECE_COLOR
            outline_color = WHITE_PIECE_OUTLINE if piece.color == WHITE else BLACK_PIECE_OUTLINE

            if piece.type == PAWN:
                pygame.draw.circle(screen_surface, fill_color, (center_x, center_y + 10), radius - 5)
                pygame.draw.circle(screen_surface, outline_color, (center_x, center_y + 10), radius - 5, 3)
                pygame.draw.circle(screen_surface, fill_color, (center_x, center_y - 10), radius - 12)
                pygame.draw.circle(screen_surface, outline_color, (center_x, center_y - 10), radius - 12, 3)

            elif piece.type == ROOK:
                rect_left = (column * TILE_SIZE) + 20
                rect_top = (row * TILE_SIZE) + 20
                pygame.draw.rect(screen_surface, fill_color, (rect_left, rect_top, 40, 40))
                pygame.draw.rect(screen_surface, outline_color, (rect_left, rect_top, 40, 40), 3)

            elif piece.type == KNIGHT:
                point_top = (center_x, center_y - 20)
                point_bottom_left = (center_x - 15, center_y + 20)
                point_bottom_right = (center_x + 15, center_y + 20)
                pygame.draw.polygon(screen_surface, fill_color, [point_top, point_bottom_left, point_bottom_right])
                pygame.draw.polygon(screen_surface, outline_color, [point_top, point_bottom_left, point_bottom_right], 3)

            elif piece.type == BISHOP:
                pygame.draw.circle(screen_surface, fill_color, (center_x, center_y), radius - 3)
                pygame.draw.circle(screen_surface, outline_color, (center_x, center_y), radius - 3, 3)
                pygame.draw.line(screen_surface, outline_color, (center_x, center_y - 15), (center_x, center_y + 15), 3)

            elif piece.type == QUEEN:
                pygame.draw.circle(screen_surface, fill_color, (center_x, center_y), radius)
                pygame.draw.circle(screen_surface, outline_color, (center_x, center_y), radius, 3)
                pygame.draw.circle(screen_surface, outline_color, (center_x, center_y), radius - 8, 2)

            elif piece.type == KING:
                pygame.draw.circle(screen_surface, fill_color, (center_x, center_y), radius)
                pygame.draw.circle(screen_surface, outline_color, (center_x, center_y), radius, 3)
                pygame.draw.line(screen_surface, outline_color, (center_x - 10, center_y), (center_x + 10, center_y), 4)
                pygame.draw.line(screen_surface, outline_color, (center_x, center_y - 10), (center_x, center_y + 10), 4)

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
                clicked_column = mouse_x // TILE_SIZE
                clicked_row = mouse_y // TILE_SIZE

                if selected_square is None:
                    clicked_piece = game_board.grid[clicked_row][clicked_column]
                    if clicked_piece and clicked_piece.color == game_board.turn:
                        selected_square = (clicked_row, clicked_column)
                else:
                    start_row, start_column = selected_square
                    available_legal_moves = game_board.get_legal_moves(start_row, start_column)
                    
                    if (clicked_row, clicked_column) in available_legal_moves:
                        game_board.make_move(selected_square, (clicked_row, clicked_column))
                        
                    selected_square = None

        draw_chessboard(screen, selected_square)
        draw_pieces(screen, game_board.grid)
        pygame.display.flip()

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()