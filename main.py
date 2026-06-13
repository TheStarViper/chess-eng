# pygbag: webgl
import pygame
import sys
import asyncio
from gamestate import ChessBoard
from graphics.pieces import *
from graphics.button import Button
from graphics.animator import PieceAnimation
from graphics.sidebar import draw_sidebar
from graphics.rightside_screen import *
from variables import *
from graphics.general_gfx import *

screen = pygame.display.set_mode(
    (WINDOW_WIDTH, WINDOW_HEIGHT), 
    pygame.NOFRAME | pygame.HWSURFACE | pygame.DOUBLEBUF
)
pygame.display.set_caption("PyChess")

clock = pygame.time.Clock()


ogbackground = pygame.image.load('assets/images/bg.jpg').convert()
background = pygame.transform.scale(ogbackground, (WINDOW_WIDTH, WINDOW_HEIGHT))
background.set_alpha(75)


PROMOTION_OVERLAY = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA).convert_alpha()
PROMOTION_OVERLAY.fill((0, 0, 0, 150))
board_surface, board_mask = initialize_chessboard()
pre_render_pieces(pieces, (249, 249, 251), (87, 85, 81),(59, 69, 54), thickness=1)

def draw_captured_bars(screen_surface, captured_white, captured_black):
    PANEL_COLOR = (30, 30, 30)
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, WHITE_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT), 0, 0, 0, 0, 5, 5)
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, BLACK_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT), 0, 0, 5, 5)
    
    def draw_mini_piece(piece, x, y): 
        MINI_SIZE = TILE_SIZE // 1.3 
        side_key = "WHITE" if piece.color == WHITE or piece.color == "w" else "BLACK"
        raw_surf = PIECE_CACHE.get((piece.type, side_key))
        if raw_surf is not None:
            scaled_surf = pygame.transform.smoothscale(raw_surf, (MINI_SIZE, MINI_SIZE))
        
            rect = scaled_surf.get_rect(center=(x, y))
            screen_surface.blit(scaled_surf, rect)

    for index, piece in enumerate(captured_white):
        column_offset = (index % 8) * (TILE_SIZE // 2)
        draw_mini_piece(piece, CAPTURE_BAR_X + 30 + column_offset, BLACK_CAPTURE_BAR_Y + CAPTURE_BAR_HEIGHT // 2 - CAPTURE_BAR_HEIGHT // 15)

    for index, piece in enumerate(captured_black):
        column_offset = (index % 8) * (TILE_SIZE // 2)
        draw_mini_piece(piece, CAPTURE_BAR_X + 30 + column_offset, WHITE_CAPTURE_BAR_Y + CAPTURE_BAR_HEIGHT // 2 - CAPTURE_BAR_HEIGHT // 15)


def draw_promotion_menu(screen_surface, turn_color, promotion_col, promotion_row,piece_color):
    button_w = TILE_SIZE
    button_h = TILE_SIZE
    cancel_btn_h = TILE_SIZE // 3
    total_h = (button_h * 4) + cancel_btn_h
    
    menu_x = BOARD_OFFSET_X + (promotion_col * button_w)
    menu_y = BOARD_OFFSET_Y if promotion_row == 0 else (BOARD_OFFSET_Y + (8 * TILE_SIZE) - total_h)
    
    screen_surface.blit(PROMOTION_OVERLAY, (0, 0))
    
    pygame.draw.rect(screen_surface, (240, 240, 240), (menu_x, menu_y, button_w, total_h), border_radius=4)
    pygame.draw.rect(screen_surface, (60, 60, 60), (menu_x, menu_y, button_w, total_h), 2, border_radius=4)
    
    mouse_x, mouse_y = pygame.mouse.get_pos()
    piece_types = [QUEEN, ROOK, BISHOP, KNIGHT]
    fill_color = WHITE_PIECE_COLOR if turn_color == WHITE else BLACK_PIECE_COLOR
    outline_color = WHITE_PIECE_OUTLINE if turn_color == WHITE else BLACK_PIECE_OUTLINE
    radius = int(TILE_SIZE * 0.4)
    
    for index, piece_type in enumerate(piece_types):
        box_y = menu_y + (index * button_h)
        btn_center_x = menu_x + (button_w // 2)
        btn_center_y = box_y + (button_h // 2)
        
        if menu_x <= mouse_x <= menu_x + button_w and box_y <= mouse_y <= box_y + button_h:
            pygame.draw.rect(screen_surface, (210, 210, 210), (menu_x + 2, box_y + 2, button_w - 4, button_h - 4), border_radius=2)
        
        if index > 0:
            pygame.draw.line(screen_surface, (200, 200, 200), (menu_x, box_y), (menu_x + button_w, box_y), 1)
            
        draw_piece(piece_type, screen_surface, fill_color, btn_center_x, btn_center_y,piece_color)
        
    cancel_y = menu_y + (button_h * 4)
    
    if menu_x <= mouse_x <= menu_x + button_w and cancel_y <= mouse_y <= cancel_y + cancel_btn_h:
        pygame.draw.rect(screen_surface, (245, 95, 95), (menu_x, cancel_y, button_w, cancel_btn_h), border_radius=3)
    else:
        pygame.draw.rect(screen_surface, (220, 80, 80), (menu_x, cancel_y, button_w, cancel_btn_h), border_radius=3)
    
    x_text = get_cached_text("X", log_font, (255, 255, 255), (220, 80, 80) if not (menu_x <= mouse_x <= menu_x + button_w and cancel_y <= mouse_y <= cancel_y + cancel_btn_h) else (245, 95, 95))
    text_rect = x_text.get_rect(center=(menu_x + (button_w // 2), cancel_y + (cancel_btn_h // 2)))
    screen_surface.blit(x_text, text_rect)


def draw_historical_pieces(screen, history_grid):
    for row in range(8):
        for col in range(8):
            data = history_grid[row][col]
            if data is not None:
                pixel_x = BOARD_OFFSET_X + (col * TILE_SIZE)
                pixel_y = BOARD_OFFSET_Y + (row * TILE_SIZE)
                
                if data["color"] == WHITE or data["color"] == "w":
                    fill = (248, 248, 248)
                    historical_side = WHITE 
                else:
                    fill = (86, 83, 82)
                    historical_side = BLACK  
                    
                radius = int(TILE_SIZE * 0.45)
                
                draw_piece(
                    data["type"], 
                    screen, 
                    fill,
                    int(pixel_x + TILE_SIZE // 2), 
                    int(pixel_y + TILE_SIZE // 2),
                    historical_side,
                    angle=0
                )


def create_sidebar_elements(width, height):
    sidebar_bg = pygame.Surface((200, height), pygame.SRCALPHA).convert_alpha()
    sidebar_bg.fill((25, 27, 29, 160))
    
    sidebar_buttons = {
        "settings": Button(width-175, height-100, 150, 50, "Settings", (27, 29, 31), (35, 37, 39), font_size=20, border=False),
        "play": Button(width-175, 120, 150, 50, "Play", (27, 29, 31), (35, 37, 39), font_size=20, border=False),
        "game": Button(width-175, 180, 150, 50, "Game", (27, 29, 31), (35, 37, 39), font_size=20, border=False),
        "puzzles": Button(width-175, 240, 150, 50, "Puzzles", (27, 29, 31), (35, 37, 39), font_size=20, border=False)
    }
    return sidebar_bg, sidebar_buttons


def create_endgame_buttons():
    center_panel_y = (WINDOW_HEIGHT - 220) // 2
    btn_w, btn_h = 160, 50
    btn_left_x = (WINDOW_WIDTH // 2) - btn_w - 20
    btn_right_x = (WINDOW_WIDTH // 2) + 20
    btn_y = center_panel_y + 120
    
    rematch_btn = Button(btn_left_x, btn_y, btn_w, btn_h, "Rematch", (50, 150, 50), (70, 190, 70))
    new_game_btn = Button(btn_right_x, btn_y, btn_w, btn_h, "New Game", (70, 70, 180), (100, 100, 230))
    return rematch_btn, new_game_btn


def create_action_buttons():
    return {
        WHITE: {
            "resign": Button(RESIGN_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Resign", (27, 29, 31), (35, 37, 39), font_size=16, action_type="resign", image_path="assets/images/resign.png"),
            "draw": Button(DRAW_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Draw", (27, 29, 31), (35, 37, 39), font_size=16, action_type="draw", image_path="assets/images/draw.png")
        },
        BLACK: {
            "resign": Button(RESIGN_BTN_X, ACTION_BTN_Y_BLACK, ACTION_BTN_W, ACTION_BTN_H, "Resign", (27, 29, 31), (35, 37, 39), font_size=16, action_type="resign", image_path="assets/images/resign.png"),
            "draw": Button(DRAW_BTN_X, ACTION_BTN_Y_BLACK, ACTION_BTN_W, ACTION_BTN_H, "Draw", (27, 29, 31), (35, 37, 39), font_size=16, action_type="draw", image_path="assets/images/draw.png")
        }
    }


def handle_pawn_promotion_click(game_board, mouse_x, mouse_y, p_row, p_col, start_row, start_col, captured_backup):
    PIECE_OPTIONS = [QUEEN, ROOK, BISHOP, KNIGHT]
    NOTATION_SUFFIXES = {QUEEN: '=Q', ROOK: '=R', BISHOP: '=B', KNIGHT: '=N'}
    
    menu_x = BOARD_OFFSET_X + (p_col * TILE_SIZE)
    button_h = TILE_SIZE
    cancel_btn_h = TILE_SIZE // 3
    total_h = (button_h * 4) + cancel_btn_h
    menu_y = BOARD_OFFSET_Y if p_row == 0 else (BOARD_OFFSET_Y + (8 * TILE_SIZE) - total_h)
    
    if not (menu_x <= mouse_x <= menu_x + TILE_SIZE and menu_y <= mouse_y <= menu_y + total_h):
        return captured_backup
        
    relative_y = mouse_y - menu_y
    if relative_y < (button_h * 4):
        chosen_upgrade = PIECE_OPTIONS[int(relative_y // button_h)]
        game_board.promote_pawn(chosen_upgrade)
        if game_board.move_history:
            last_entry = game_board.move_history[-1]
            suffix = NOTATION_SUFFIXES.get(chosen_upgrade, '')
            if isinstance(last_entry, str):
                game_board.move_history[-1] += suffix
            elif isinstance(last_entry, dict) and "notation" in last_entry:
                last_entry["notation"] += suffix
        global game_is_over, game_over_reason
        game_is_over, game_over_reason = check_game_over_states(game_board)
    else:
        moving_pawn = game_board.grid[p_row][p_col]
        if moving_pawn:
            game_board.grid[start_row][start_col] = moving_pawn
            game_board.grid[p_row][p_col] = captured_backup
            game_board.turn = moving_pawn.color
            if game_board.move_history:
                game_board.move_history.pop()
    
    game_board.promotion_required = False
    return None


def handle_board_square_click(game_board, clicked_row, clicked_col, selected_square, selected_piece_moves, board_offset):
    if selected_square is None:
        clicked_piece = game_board.grid[clicked_row][clicked_col]
        if clicked_piece and clicked_piece.color == game_board.turn:
            return (clicked_row, clicked_col), game_board.get_safe_legal_moves(clicked_row, clicked_col), None, None
        return None, [], None, None

    start_row, start_col = selected_square
    clicked_piece = game_board.grid[clicked_row][clicked_col]
    
    if clicked_piece and clicked_piece.color == game_board.turn:
        return (clicked_row, clicked_col), game_board.get_safe_legal_moves(clicked_row, clicked_col), None, None
        
    if (clicked_row, clicked_col) not in selected_piece_moves:
        return None, [], None, None

    moving_piece = game_board.grid[start_row][start_col]
    captured_backup = game_board.grid[clicked_row][clicked_col]
    
    hidden_data = {
        "row": clicked_row,
        "col": clicked_col,
        "piece": moving_piece
    }
    
    animation = PieceAnimation(
        moving_piece.type, moving_piece.color,
        (start_row, start_col), (clicked_row, clicked_col),
        TILE_SIZE, board_offset, speed=0.2
    )
    
    if moving_piece.type == 1 and start_col != clicked_col and captured_backup is None:
        game_board.grid[start_row][clicked_col] = None

    game_board.make_move(selected_square, (clicked_row, clicked_col))
    return None, [], hidden_data, animation


def handle_scroll_motion(event, game_board, max_viewable_height, max_scroll_val):
    relative_y = event.pos[1] - (PANEL_Y - 5)
    scroll_ratio = relative_y / max_viewable_height
    scroll_ratio = max(0.0, min(1.0, scroll_ratio))
    return int(scroll_ratio * max_scroll_val)


def build_mock_history_grid(viewing_grid):
    class MockPiece:
        def __init__(self, p_type, p_color):
            self.type = p_type
            self.color = p_color

    temp_object_grid = [[None for _ in range(8)] for _ in range(8)]
    for r in range(8):
        for c in range(8):
            data = viewing_grid[r][c]
            if data is not None:
                if isinstance(data, dict):
                    raw_color = data.get("color")
                    p_type = data.get("type")
                else:
                    raw_color = getattr(data, "color", WHITE)
                    p_type = getattr(data, "type", 1)
                    
                actual_color = WHITE if raw_color in [WHITE, "w"] else BLACK
                temp_object_grid[r][c] = MockPiece(p_type, actual_color)
    return temp_object_grid

def determine_checked_king(game_board, selected_history_index, viewing_grid):
    if selected_history_index is not None:
        temp_grid = build_mock_history_grid(viewing_grid)
        original_live_grid = game_board.grid
        game_board.grid = temp_grid
        
        pos = None
        if game_board.is_in_check(WHITE):
            pos = ChessBoard.get_king_position(temp_grid, WHITE)
        elif game_board.is_in_check(BLACK):
            pos = ChessBoard.get_king_position(temp_grid, BLACK)
            
        game_board.grid = original_live_grid
        return pos
        
    if game_board.is_in_check(WHITE):
        return ChessBoard.get_king_position(game_board.grid, WHITE)
    if game_board.is_in_check(BLACK):
        return ChessBoard.get_king_position(game_board.grid, BLACK)
    return None


def draw_live_pieces(screen, game_board, hidden_piece_data):
    omit_square = (hidden_piece_data["row"], hidden_piece_data["col"]) if hidden_piece_data else None
    for row in range(8):
        for col in range(8):
            if omit_square == (row, col):
                continue
            piece = game_board.grid[row][col]
            if piece:
                pixel_x = BOARD_OFFSET_X + (col * TILE_SIZE) + TILE_SIZE // 2
                pixel_y = BOARD_OFFSET_Y + (row * TILE_SIZE) + TILE_SIZE // 2
                fill, outline = ((248, 248, 248), (45, 45, 45)) if piece.color == WHITE else ((86, 83, 82), (25, 25, 25))
                draw_piece(piece.type, screen, fill, int(pixel_x), int(pixel_y), piece.color)


def resolve_history_state(game_board, selected_history_index):
    actual_moves_count = len(game_board.move_history) - 1
    if selected_history_index is None or selected_history_index >= actual_moves_count:
        return game_board.last_move, True, game_board.grid

    target_idx = selected_history_index + 2
    if target_idx >= len(game_board.move_history):
        return game_board.last_move, True, game_board.grid 

    display_state = game_board.move_history[target_idx]
    if isinstance(display_state, dict) and "grid" in display_state:
        return display_state.get("last_move", None), False, display_state["grid"]
        
    return game_board.last_move, True, game_board.grid 

async def main():
    global LOG_SCROLL_Y, is_dragging_scroll
    width, height = pygame.display.get_window_size()

    sidebar_bg, sidebar_buttons = create_sidebar_elements(width, height)
    rematch_btn, new_game_btn = create_endgame_buttons()
    action_buttons = create_action_buttons()

    game_board = ChessBoard()
    selected_square = None
    is_game_running = True
    board_offset = (BOARD_OFFSET_X, BOARD_OFFSET_Y)

    captured_piece_backup = None
    active_animation = None
    hidden_piece_data = None
    game_is_over = False

    selected_history_index = None
    selected_piece_moves = []
    promotion_col, promotion_row = 0, 0
    start_row, start_column = 0, 0
    
    STATIC_INTERFACE_SURFACE = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT)).convert()
    STATIC_INTERFACE_SURFACE.blit(background, (0, 0))
    draw_rightside(STATIC_INTERFACE_SURFACE, other_log_font)
    draw_sidebar(STATIC_INTERFACE_SURFACE, sidebar_bg, sidebar_buttons)

    while is_game_running:
        if not DEBUGMODE:
            clock.tick(60) 
        mouse_pos = pygame.mouse.get_pos()
        current_turn = game_board.turn
        actual_moves_count = len(game_board.move_history) - 1

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                is_game_running = False
                continue
                
            if event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE and DEBUGMODE:
                is_game_running = False
                continue
            
            if event.type == pygame.MOUSEBUTTONUP and event.button == 1:
                is_dragging_scroll = False
                continue
                
            if event.type == pygame.MOUSEMOTION and is_dragging_scroll:
                container_height = TILE_SIZE * 6
                max_viewable_height = container_height - 20
                actual_moves = game_board.move_history[1:]
                total_content_height = ((len(actual_moves) + 1) // 2) * ROW_HEIGHT
                max_scroll_val = total_content_height - max_viewable_height
                if max_scroll_val > 0:
                    LOG_SCROLL_Y = handle_scroll_motion(event, game_board, max_viewable_height, max_scroll_val)
                continue

            if event.type != pygame.MOUSEBUTTONDOWN:
                continue

            LOG_SCROLL_Y, is_dragging_scroll, should_continue = handle_move_log_scrolling(
                event, game_board.move_history, LOG_SCROLL_Y, is_dragging_scroll, DEBUGMODE
            )
            if should_continue:
                continue
                
            clicked_log_idx = check_move_log_click(game_board.move_history, mouse_pos)
            if clicked_log_idx is not None:
                selected_history_index = clicked_log_idx
                continue
                
            clicked_nav = check_nav_bar_click(mouse_pos)
            if clicked_nav is not None:
                match clicked_nav:
                    case "FIRST":
                        selected_history_index = 0 if actual_moves_count > 0 else None
                    case "PREV" if actual_moves_count > 0:
                        idx = selected_history_index if selected_history_index is not None else actual_moves_count - 1
                        selected_history_index = max(0, idx - 1)
                    case "NEXT" if selected_history_index is not None:
                        selected_history_index = None if selected_history_index >= actual_moves_count - 1 else selected_history_index + 1
                    case "LATEST":
                        selected_history_index = None
                continue

            if selected_history_index is not None:
                selected_history_index = None
                
            if active_animation and active_animation.is_active:
                continue
                
            mouse_x, mouse_y = mouse_pos

            if game_board.promotion_required:
                captured_piece_backup = handle_pawn_promotion_click(
                    game_board, mouse_x, mouse_y, promotion_row, promotion_col, start_row, start_column, captured_piece_backup
                )
                continue
            
            if game_is_over and (rematch_btn.is_clicked(mouse_pos, event.type) or new_game_btn.is_clicked(mouse_pos, event.type)):
                game_board = ChessBoard()
                selected_square = None
                game_is_over = False
                continue
            
            if not game_is_over:
                if action_buttons[current_turn]["resign"].is_clicked(mouse_pos, event.type):
                    game_is_over = True
                    game_board.game_over_reason = "WHITE_RESIGN" if current_turn == WHITE else "BLACK_RESIGN"
                    selected_square = None
                    continue
                elif action_buttons[current_turn]["draw"].is_clicked(mouse_pos, event.type):
                    game_is_over = True
                    game_board.game_over_reason = "DRAW"
                    selected_square = None
                    continue
            clicked_column = (mouse_x - BOARD_OFFSET_X) // TILE_SIZE
            clicked_row = (mouse_y - BOARD_OFFSET_Y) // TILE_SIZE

            if 0 <= clicked_row < 8 and 0 <= clicked_column < 8:
                if selected_square is not None:
                    start_row, start_column = selected_square
                    
                selected_square, selected_piece_moves, hidden_piece_data, active_animation = handle_board_square_click(
                    game_board, clicked_row, clicked_column, selected_square, selected_piece_moves, board_offset
                )
                if game_board.promotion_required:
                    promotion_col = clicked_column
                    promotion_row = clicked_row
                game_is_over, game_over_reason = check_game_over_states(game_board)
                if game_is_over:
                    game_board.game_over_reason = game_over_reason
                    if DEBUGMODE:
                        print(game_over_reason) 
            else:
                selected_square = None

        if game_is_over: 
            rematch_btn.is_hover(mouse_pos)
            new_game_btn.is_hover(mouse_pos)
        else:
            action_buttons[current_turn]["resign"].is_hover(mouse_pos)
            action_buttons[current_turn]["draw"].is_hover(mouse_pos)
            
        in_animation = active_animation is not None and active_animation.is_active
        screen.blit(STATIC_INTERFACE_SURFACE, (0, 0))
        draw_move_log_table(screen, game_board.move_history, log_font, selected_history_index, LOG_SCROLL_Y)
        draw_nav_bar(screen, log_font, game_board.move_history)
        
        viewing_last_move, use_live, viewing_grid = resolve_history_state(game_board, selected_history_index)
        checked_king_pos = None if in_animation else determine_checked_king(game_board, selected_history_index, viewing_grid)

        screen.set_clip(pygame.Rect(BOARD_OFFSET_X, BOARD_OFFSET_Y, board_size, board_size))
        draw_chessboard(screen, selected_square, viewing_last_move, in_animation, checked_king_pos, board_surface, board_mask)
        
        if not use_live:
            draw_historical_pieces(screen, viewing_grid)
        else:
            draw_live_pieces(screen, game_board, hidden_piece_data)

            if active_animation and active_animation.is_active:
                active_animation.update()
                fill, outline = ((248, 248, 248), (45, 45, 45)) if active_animation.color == WHITE else ((86, 83, 82), (25, 25, 25))
                draw_piece(active_animation.piece_type, screen, fill, int(active_animation.current_x), int(active_animation.current_y), active_animation.color, angle=0)
            else:
                hidden_piece_data = None
                active_animation = None

        if selected_square is not None and selected_history_index is None:
            draw_legal_moves(screen, selected_piece_moves, game_board.grid, TILE_SIZE, (BOARD_OFFSET_X, BOARD_OFFSET_Y))
            
        screen.set_clip(None)
        draw_captured_bars(screen, game_board.captured_white, game_board.captured_black)

        if DEBUGMODE:
            clock.tick()
            fps_surface = get_cached_text(str(int(clock.get_fps())), log_font, pygame.Color("green"), (19, 21, 23))
            screen.blit(fps_surface, (10, 10))
            
        if not game_is_over:
            action_buttons[current_turn]["resign"].draw(screen)
            action_buttons[current_turn]["draw"].draw(screen)

        if game_board.promotion_required:
            draw_promotion_menu(screen, game_board.turn, promotion_col, promotion_row, piece.color)

        if game_is_over:
            draw_game_over_screen(screen, game_board.turn, [rematch_btn, new_game_btn], game_board, font_title)
        
        pygame.display.flip()
        await asyncio.sleep(0)
        
    pygame.quit()
    sys.exit()
    
if __name__ == "__main__":
    asyncio.run(main())