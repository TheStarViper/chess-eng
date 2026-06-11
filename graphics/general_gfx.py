import pygame
from variables import *
from graphics.pieces import draw_piece
from gamestate import *

def draw_game_over_screen(screen_surface, losing_color, game_over_buttons, game_board, font):
    
    overlay = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
    overlay.fill((0, 0, 0, 180))
    screen_surface.blit(overlay, (0, 0))

    pygame.draw.rect(screen_surface, (40, 40, 40), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT), 0, 0, 15, 15, 15, 15)
    pygame.draw.rect(screen_surface, (230, 50, 50), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT), 4, 15)

    reason = getattr(game_board, "game_over_reason", "CHECKMATE")

    match reason:
        case "WHITE_RESIGN":
            end_text = "BLACK WINS BY RESIGNATION!"
        case "BLACK_RESIGN":
            end_text = "WHITE WINS BY RESIGNATION!"
        case "DRAW":
            end_text = "GAME DRAWN BY AGREEMENT"
        case "FIFTY_MOVE_RULE" | "FIFTY_MOVES" | "50_MOVE_RULE":
            end_text = "DRAW! (50-Move Rule Reached)"
        case "STALEMATE":
            end_text = "DRAW! (Stalemate)"
        case "REPETITION" | "THREEFOLD_REPETITION" | "DRAW_REPETITION":
            end_text = "DRAW! (Threefold Repetition)"
        case "INSUFFICIENT_MATERIAL" | "DEAD_POSITION":
            end_text = "DRAW! (Insufficient Material)"
        case "CHECKMATE" | "MATE":
            end_text = "BLACK WINS BY MATE!" if losing_color == WHITE else "WHITE WINS BY MATE!"
        case _:
            end_text = f"GAME OVER ({reason})"

    title_surface = font.render(end_text, True, (255, 255, 255))

    if title_surface.get_width() > GAME_OVER_WIDTH - 20:
        title_surface = log_font.render(end_text, True, (255, 255, 255))

    screen_surface.blit(title_surface, (GAME_OVER_X + (GAME_OVER_WIDTH - title_surface.get_width()) // 2, GAME_OVER_Y + 30))

    for button in game_over_buttons:
        button.draw(screen_surface)
        
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

def draw_legal_moves(surface, legal_moves, grid,tile_size, board_offset):

    mouse_x, mouse_y = pygame.mouse.get_pos()
    
    offset_x, offset_y = board_offset
    
    standard_radius = int(tile_size * 0.15)  
    hover_radius = int(tile_size * 0.18)     
    capture_radius = int(tile_size * 0.45)   
    
    alpha_surface = pygame.Surface((surface.get_width(), surface.get_height()), pygame.SRCALPHA)

    for move in legal_moves:
        row, column = move
        
        tile_left = offset_x + (column * tile_size)
        tile_top = offset_y + (row * tile_size)
        
        center_x = tile_left + (tile_size // 2)
        center_y = tile_top + (tile_size // 2)
        
        if tile_left <= mouse_x < tile_left + tile_size and tile_top <= mouse_y < tile_top + tile_size:
            current_radius = hover_radius
        else:
            current_radius = standard_radius
        is_capture = grid[row][column] is not None
        
        if is_capture:
            thickness = max(2, int(tile_size * 0.06))
            pygame.draw.circle(alpha_surface, PREMOVE_HIGHLIGHT_COLOR, (center_x, center_y), capture_radius, thickness)
        else:
            pygame.draw.circle(alpha_surface, PREMOVE_HIGHLIGHT_COLOR, (center_x, center_y), current_radius)

    surface.blit(alpha_surface, (0, 0))

