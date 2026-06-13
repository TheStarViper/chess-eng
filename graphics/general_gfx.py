import pygame
from variables import *
from graphics.pieces import draw_piece
from gamestate import *
from graphics.rightside_screen import get_cached_text
from gameover import *

def draw_game_over_screen(screen_surface, losing_color, game_over_buttons, game_board, font):
    overlay = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
    overlay.fill((0, 0, 0, 180))
    screen_surface.blit(overlay, (0, 0))

    pygame.draw.rect(screen_surface, (40, 40, 40), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT), 0, 15)
    pygame.draw.rect(screen_surface, (230, 50, 50), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT), 4, 15)

    reason = getattr(game_board, "game_over_reason", "CHECKMATE")
    match reason:
        case "WHITE_RESIGN":          end_text = "BLACK WINS BY RESIGNATION!"
        case "BLACK_RESIGN":          end_text = "WHITE WINS BY RESIGNATION!"
        case "DRAW":                  end_text = "GAME DRAWN BY AGREEMENT"
        case "FIFTY_MOVE_RULE":       end_text = "DRAW! (50-Move Rule Reached)"
        case "STALEMATE":             end_text = "DRAW! (Stalemate)"
        case "THREEFOLD_REPETITION":  end_text = "DRAW! (Threefold Repetition)"
        case "INSUFFICIENT_MATERIAL": end_text = "DRAW! (Insufficient Material)"
        case "CHECKMATE":
            end_text = "BLACK WINS BY MATE!" if losing_color == WHITE else "WHITE WINS BY MATE!"
        case _:
            end_text = f"GAME OVER ({reason})"

    if not game_board.end_game_sound_played:
        if "DRAW" in reason or reason in ("STALEMATE", "FIFTY_MOVE_RULE", "THREEFOLD_REPETITION", "INSUFFICIENT_MATERIAL"):
            draw_sound.play()
        else:
            win_sound.play()
        game_board.end_game_sound_played = True
    title_surface = font.render(end_text, True, (255, 255, 255))
    max_allowed_width = GAME_OVER_WIDTH - 40

    if title_surface.get_width() > max_allowed_width:
        scale_ratio = max_allowed_width / title_surface.get_width()
        new_w = int(title_surface.get_width() * scale_ratio)
        new_h = int(title_surface.get_height() * scale_ratio)
        title_surface = pygame.transform.smoothscale(title_surface, (new_w, new_h))

    screen_surface.blit(title_surface, (GAME_OVER_X + (GAME_OVER_WIDTH - title_surface.get_width()) // 2, GAME_OVER_Y + 30))

    for button in game_over_buttons:
        button.draw(screen_surface)

def check_game_over_states(game_board):
    current_turn = game_board.turn
    in_check = game_board.is_in_check(current_turn)
    has_moves = has_legal_moves(game_board, current_turn)

    if in_check and not has_moves:
        return True, "CHECKMATE"

    if not in_check and not has_moves:
        return True, "STALEMATE"

    if is_insufficient_material(game_board.grid):
        return True, "INSUFFICIENT_MATERIAL"

    if getattr(game_board, "halfmove_clock", 0) >= 100:
        return True, "FIFTY_MOVE_RULE"

    if check_threefold_repetition(game_board):
        return True, "THREEFOLD_REPETITION"

    return False, ""


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

def draw_legal_moves(surface, legal_moves, grid, tile_size, board_offset):
    mouse_x, mouse_y = pygame.mouse.get_pos()
    offset_x, offset_y = board_offset
    
    hovered_col = (mouse_x - offset_x) // tile_size
    hovered_row = (mouse_y - offset_y) // tile_size
    current_hovered_tile = (hovered_row, hovered_col)

    if not hasattr(draw_legal_moves, "last_hovered_tile"):
        draw_legal_moves.last_hovered_tile = None

    if current_hovered_tile in legal_moves:
        if current_hovered_tile != draw_legal_moves.last_hovered_tile:
            hover_sound.play()
            draw_legal_moves.last_hovered_tile = current_hovered_tile
    else:
        draw_legal_moves.last_hovered_tile = None

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
        
        if row == hovered_row and column == hovered_col:
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



def initialize_chessboard():
    board_mask = pygame.Surface((board_size, board_size), pygame.SRCALPHA).convert_alpha()
    board_mask.fill((0, 0, 0, 0))
    pygame.draw.rect(board_mask, (255, 255, 255, 255), (0, 0, board_size, board_size), border_radius=10)

    STATIC_BOARD_SURFACE = pygame.Surface((board_size, board_size)).convert()

    for row in range(8):
        for column in range(8):
            square_color = LIGHT_SQUARE_COLOR if (row + column) % 2 == 0 else DARK_SQUARE_COLOR
            pygame.draw.rect(STATIC_BOARD_SURFACE, square_color, (column * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE))
    return STATIC_BOARD_SURFACE, board_mask
    

def draw_chessboard(screen_surface, selected_square, last_move, in_animation, checked_king_pos, board_surface1,board_mask):
    board_surface = board_surface1.copy()

    if last_move is not None:
        start_pos, end_pos = last_move
        pygame.draw.rect(board_surface, LAST_MOVE_HIGHLIGHT_COLOR, (start_pos[1]*TILE_SIZE, start_pos[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))
        pygame.draw.rect(board_surface, LAST_MOVE_HIGHLIGHT_COLOR, (end_pos[1]*TILE_SIZE, end_pos[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))

    if not in_animation and checked_king_pos is not None:
        pygame.draw.rect(board_surface, CHECK_INDICATOR_COLOR, (checked_king_pos[1]*TILE_SIZE, checked_king_pos[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))
    
    if selected_square is not None:
        pygame.draw.rect(board_surface, SELECTED_HIGHLIGHT_COLOR, (selected_square[1]*TILE_SIZE, selected_square[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))
        
    for row in range(8):
        for column in range(8):
            is_light_square = (row + column) % 2 == 0
            text_color = DARK_SQUARE_COLOR if is_light_square else LIGHT_SQUARE_COLOR
            square_bg = LIGHT_SQUARE_COLOR if is_light_square else DARK_SQUARE_COLOR
            
            local_x = column * TILE_SIZE
            local_y = row * TILE_SIZE

            if column == 0:
                rank_text = str(8 - row)
                num_surf = get_cached_text(rank_text, coord_font, text_color, square_bg)
                num_rect = num_surf.get_rect(topright=(local_x + 16, local_y + 4))
                board_surface.blit(num_surf, num_rect)
                
            if row == 7:
                file_text = chr(ord('a') + column)
                char_surf = coord_font.render(file_text, True, text_color)
                char_rect = char_surf.get_rect(bottomleft=(local_x + 4, local_y + TILE_SIZE - 2))
                board_surface.blit(char_surf, char_rect)
                
    board_surface.blit(board_mask, (0, 0), special_flags=pygame.BLEND_RGBA_MIN)
    screen_surface.blit(board_surface, (BOARD_OFFSET_X, BOARD_OFFSET_Y))

