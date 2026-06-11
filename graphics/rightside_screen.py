import pygame
from variables import *

TEXT_CACHE = {}
def get_cached_text(text_string, font_object, color, bg_color=None):
    cache_key = f"{text_string}_{color}_{bg_color}"
    if cache_key not in TEXT_CACHE:
        TEXT_CACHE[cache_key] = font_object.render(text_string, True, color, bg_color)
    return TEXT_CACHE[cache_key]

def draw_rightside(screen,font):
    rect = (LEFT_SIDE_BUFFER+TILE_SIZE*8+35,BOARD_OFFSET_Y,TILE_SIZE*5,TILE_SIZE*8)

    pygame.draw.rect(screen,
                    (25, 27, 29),
                    rect,
                    border_bottom_left_radius=10,
                    border_top_left_radius=10,
                    border_bottom_right_radius=10,
                    border_top_right_radius=10)
    
    pygame.draw.rect(screen,
                    (31, 34, 33),
                    rect,
                    border_radius=10,
                    width = 2)
    

    text_surface = font.render("Game Log", True, (255,255,255))
    screen.blit(text_surface, (rect[0]+15, rect[1]))

def draw_nav_bar(screen, font, move_history):
    container_width = TILE_SIZE * 5 - 20
    nav_y = PANEL_Y - 50
    nav_rect = pygame.Rect(PANEL_X - 10, nav_y, container_width, 40)
    
    pygame.draw.rect(screen, (25, 27, 29), nav_rect, border_radius=5)
    
    btn_w = container_width // 4
    icons = ["|<", "<", ">", ">|"]
    mouse_pos = pygame.mouse.get_pos()
    
    any_nav_hovered = False

    for i, icon in enumerate(icons):
        btn_x = (PANEL_X - 10) + (i * btn_w)
        btn_rect = pygame.Rect(btn_x + 2, nav_y + 4, btn_w - 4, 32)
        
        if btn_rect.collidepoint(mouse_pos):
            btn_color = (56, 60, 65)  
            any_nav_hovered = True
        else:
            btn_color = (32, 34, 37)  
            
        pygame.draw.rect(screen, btn_color, btn_rect, border_radius=4)
        
        text_surf = get_cached_text(icon, font, (248, 248, 248), btn_color)
        screen.blit(text_surf, text_surf.get_rect(center=btn_rect.center))
        
    if any_nav_hovered:
        pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_HAND)


def check_nav_bar_click(mouse_pos):
    container_width = TILE_SIZE * 5 - 20
    nav_y = PANEL_Y - 50
    
    if not (PANEL_X - 10 <= mouse_pos[0] <= PANEL_X - 10 + container_width and nav_y <= mouse_pos[1] <= nav_y + 40):
        return None
        
    btn_w = container_width // 4
    click_x = mouse_pos[0] - (PANEL_X - 10)
    btn_index = int(click_x // btn_w)
    
    actions = ["FIRST", "PREV", "NEXT", "LATEST"]
    if 0 <= btn_index < len(actions):
        return actions[btn_index]
    return None

def draw_move_log_table(screen, move_history, font, selected_idx, scroll_y):
    container_width = TILE_SIZE * 5 - 20
    container_height = TILE_SIZE * 6
    container_rect = pygame.Rect(PANEL_X - 10, PANEL_Y - 10, container_width, container_height)
    
    pygame.draw.rect(screen, (25, 27, 29), container_rect, border_radius=5)
    
    actual_moves = move_history[1:] 
    total_pairs = (len(actual_moves) + 1) // 2
    actual_moves_count = len(actual_moves)

    if selected_idx is None and actual_moves_count > 0:
        active_outline_idx = actual_moves_count - 1
    else:
        active_outline_idx = selected_idx

    ROW_COLOR_EVEN = (32, 34, 37)   
    ROW_COLOR_ODD = (40, 43, 47)    
    HOVER_LIGHT_COLOR = (56, 60, 65)     
    OUTLINE_COLOR = (120, 180, 120)      
    button_height = ROW_HEIGHT - 4
    
    mouse_pos = pygame.mouse.get_pos()
    any_button_hovered = False

    def get_move_string(move_obj):
        if isinstance(move_obj, str):
            return move_obj.upper()
        if isinstance(move_obj, dict):
            if "notation" in move_obj:
                return move_obj["notation"].upper()
            elif "text" in move_obj:
                return move_obj["text"].upper()
        return "??"

    screen.set_clip(container_rect)

    for i in range(total_pairs):
        y_pos = PANEL_Y + (i * ROW_HEIGHT) - scroll_y
        
        if y_pos + ROW_HEIGHT < PANEL_Y - 10 or y_pos > PANEL_Y + container_height:
            continue
            
        row_bg_color = ROW_COLOR_EVEN if i % 2 == 0 else ROW_COLOR_ODD
        row_strip_rect = pygame.Rect(PANEL_X - 5, y_pos, container_width - 30, ROW_HEIGHT)
        pygame.draw.rect(screen, row_bg_color, row_strip_rect, border_radius=3)
        
        num_text = font.render(f"{i + 1}.", True, (140, 145, 150))
        num_rect = num_text.get_rect(midleft=(PANEL_X, y_pos + (ROW_HEIGHT // 2)))
        screen.blit(num_text, num_rect)
        
        white_idx = i * 2
        if white_idx < len(actual_moves):
            w_move_rect = pygame.Rect(PANEL_X + 35, y_pos + 2, COL_WIDTH, button_height)
            
            if w_move_rect.collidepoint(mouse_pos) and container_rect.collidepoint(mouse_pos):
                w_btn_color = HOVER_LIGHT_COLOR
                any_button_hovered = True
            else:
                w_btn_color = row_bg_color

            pygame.draw.rect(screen, w_btn_color, w_move_rect, border_radius=3)
            
            if active_outline_idx == white_idx:
                pygame.draw.rect(screen, OUTLINE_COLOR, w_move_rect, 2, border_radius=3)

            w_notation = get_move_string(actual_moves[white_idx])
            w_text = font.render(w_notation, True, (248, 248, 248))
            screen.blit(w_text, w_text.get_rect(center=w_move_rect.center))

        black_idx = i * 2 + 1
        if black_idx < len(actual_moves):
            b_move_rect = pygame.Rect(PANEL_X + 85 + COL_WIDTH + 5, y_pos + 2, COL_WIDTH, button_height)
            
            if b_move_rect.collidepoint(mouse_pos) and container_rect.collidepoint(mouse_pos):
                b_btn_color = HOVER_LIGHT_COLOR
                any_button_hovered = True
            else:
                b_btn_color = row_bg_color

            pygame.draw.rect(screen, b_btn_color, b_move_rect, border_radius=3)
            
            if active_outline_idx == black_idx:
                pygame.draw.rect(screen, OUTLINE_COLOR, b_move_rect, 2, border_radius=3)

            b_notation = get_move_string(actual_moves[black_idx])
            b_text = font.render(b_notation, True, (248, 248, 248))
            screen.blit(b_text, b_text.get_rect(center=b_move_rect.center))

    screen.set_clip(None)

    max_viewable_height = container_height - 20
    total_content_height = total_pairs * ROW_HEIGHT

    if total_content_height > max_viewable_height:
        scrollbar_x = PANEL_X + container_width - 25
        thumb_height = max(20, int((max_viewable_height / total_content_height) * max_viewable_height))
        max_scroll_val = total_content_height - max_viewable_height
        
        scroll_ratio = scroll_y / max_scroll_val if max_scroll_val > 0 else 0
        thumb_y = (PANEL_Y - 5) + int(scroll_ratio * (max_viewable_height - thumb_height))
        
        pygame.draw.rect(screen, (35, 38, 41), (scrollbar_x, PANEL_Y - 5, 8, max_viewable_height), border_radius=4)
        pygame.draw.rect(screen, (90, 95, 100), (scrollbar_x, thumb_y, 8, thumb_height), border_radius=4)

    if any_button_hovered:
        pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_HAND)
    else:
        pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_ARROW)

    return scroll_y

def check_move_log_click(move_history, mouse_pos):
    actual_moves = move_history[1:]
    total_pairs = (len(actual_moves) + 1) // 2
    button_height = ROW_HEIGHT - 4

    for i in range(total_pairs):
        y_pos = PANEL_Y + (i * ROW_HEIGHT) - LOG_SCROLL_Y
        
        w_move_rect = pygame.Rect(PANEL_X + 35, y_pos + 2, COL_WIDTH, button_height)
        if w_move_rect.collidepoint(mouse_pos):
            return i * 2
            
        black_idx = i * 2 + 1
        if black_idx < len(actual_moves):
            b_move_rect = pygame.Rect(PANEL_X + 85 + COL_WIDTH + 5, y_pos + 2, COL_WIDTH, button_height)
            if b_move_rect.collidepoint(mouse_pos):
                return black_idx
                
    return None