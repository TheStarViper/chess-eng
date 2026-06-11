import pygame
from variables import *
from graphics.pieces import draw_piece
from gamestate import WHITE,BLACK
from main import get_cached_text

def draw_game_over_screen(screen_surface, losing_color, game_over_buttons, game_board, font):
    
    overlay = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
    overlay.fill((0, 0, 0, 180))
    screen_surface.blit(overlay, (0, 0))

    
    pygame.draw.rect(screen_surface, (40, 40, 40), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT),0,0,15,15,15,15)
    pygame.draw.rect(screen_surface, (230, 50, 50), (GAME_OVER_X, GAME_OVER_Y, GAME_OVER_WIDTH, GAME_OVER_HEIGHT),4,15)

    match game_board.game_over_reason:
        case "WHITE_RESIGN":
            end_text = "BLACK WINS BY RESIGNATION!"
        case "BLACK_RESIGN":
            end_text = "WHITE WINS BY RESIGNATION!"
        case "DRAW":
            end_text = "GAME DRAWN BY AGREEMENT"
        case "FIFTY_MOVE_RULE":
            end_text = "DRAW! (50-Move Rule Reached)"
        case "STALEMATE":
            end_text = "DRAW! (Stalemate)"
        case "REPETITION":
            end_text = "DRAW! (Threefold Repetition)"
        case "CHECKMATE":
            end_text = "BLACK WINS BY MATE!" if losing_color == WHITE else "WHITE WINS BY MATE!"
        case _:
            end_text = "GAME OVER (ERROR)"

    

    title_surface = font.render(end_text, True, (255, 255, 255))

    
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

def draw_move_log_table(screen, move_history, mouse_pos, font):
    global LOG_SCROLL_Y, is_dragging_scroll

    container_width = TILE_SIZE * 5 - 20
    container_height = TILE_SIZE * 6
    container_rect = pygame.Rect(PANEL_X - 10, PANEL_Y - 10, container_width, container_height)
    pygame.draw.rect(screen, (25, 27, 29), container_rect, border_radius=5)
    
    actual_moves = move_history[1:] 
    total_pairs = (len(actual_moves) + 1) // 2
    
    total_content_height = total_pairs * ROW_HEIGHT
    max_viewable_height = container_height - 20 
    
    needs_scroll = total_content_height > max_viewable_height
    
    if needs_scroll:
        max_scroll = total_content_height - max_viewable_height
        LOG_SCROLL_Y = max(0, min(LOG_SCROLL_Y, max_scroll))
    else:
        LOG_SCROLL_Y = 0


    view_surface = pygame.Surface((container_width - 20, max_viewable_height))
    view_surface.fill((25, 27, 29)) 
    
    rel_mouse_x = mouse_pos[0] - (PANEL_X - 10 + 10)
    rel_mouse_y = mouse_pos[1] - (PANEL_Y - 10 + 10)
    
    hovered_index = None
    button_height = ROW_HEIGHT - 4

    for i in range(total_pairs):
        y_pos = (i * ROW_HEIGHT) - LOG_SCROLL_Y
        
        if y_pos + ROW_HEIGHT < 0 or y_pos > max_viewable_height:
            continue
            
        row_bg_color = (32, 34, 37) if i % 2 == 0 else (40, 43, 47)
        row_strip_rect = pygame.Rect(0, y_pos, container_width - 40, ROW_HEIGHT)
        pygame.draw.rect(view_surface, row_bg_color, row_strip_rect, border_radius=3)
        
        num_text = get_cached_text(f"{i + 1}.", font, (140, 145, 150), row_bg_color)
        num_rect = num_text.get_rect(midleft=(5, y_pos + (ROW_HEIGHT // 2)))
        view_surface.blit(num_text, num_rect)
        
        white_idx = i * 2
        w_move_rect = pygame.Rect(40, y_pos + 2, COL_WIDTH, button_height)
        w_btn_color = row_bg_color

        if w_move_rect.collidepoint((rel_mouse_x, rel_mouse_y)) and container_rect.collidepoint(mouse_pos):
            w_btn_color = (56, 60, 65)
            hovered_index = white_idx + 1 

        pygame.draw.rect(view_surface, w_btn_color, w_move_rect, border_radius=3)
        w_text = get_cached_text(actual_moves[white_idx]["notation"].upper(), font, (248, 248, 248), w_btn_color)
        view_surface.blit(w_text, w_text.get_rect(center=w_move_rect.center))

        black_idx = i * 2 + 1
        if black_idx < len(actual_moves):
            b_move_rect = pygame.Rect(45 + COL_WIDTH + 5, y_pos + 2, COL_WIDTH, button_height)
            b_btn_color = row_bg_color

            if b_move_rect.collidepoint((rel_mouse_x, rel_mouse_y)) and container_rect.collidepoint(mouse_pos):
                b_btn_color = (56, 60, 65)
                hovered_index = black_idx + 1

            pygame.draw.rect(view_surface, b_btn_color, b_move_rect, border_radius=3)
            b_text = get_cached_text(actual_moves[black_idx]["notation"].upper(), font, (248, 248, 248), b_btn_color)
            view_surface.blit(b_text, b_text.get_rect(center=b_move_rect.center))

    screen.blit(view_surface, (PANEL_X, PANEL_Y))

    if needs_scroll:
        scrollbar_w = 6
        scrollbar_x = PANEL_X + container_width - 25
        
        visible_ratio = max_viewable_height / total_content_height
        handle_height = max(20, int(max_viewable_height * visible_ratio))
        
        scroll_ratio = LOG_SCROLL_Y / max_scroll
        travel_distance = max_viewable_height - handle_height
        handle_y = PANEL_Y + int(scroll_ratio * travel_distance)
        
        handle_rect = pygame.Rect(scrollbar_x, handle_y, scrollbar_w, handle_height)
        
        track_hover = handle_rect.collidepoint(mouse_pos) or is_dragging_scroll
        bar_color = (100, 105, 110) if track_hover else (60, 63, 67)
        
        pygame.draw.rect(screen, bar_color, handle_rect, border_radius=3)
            
    return hovered_index