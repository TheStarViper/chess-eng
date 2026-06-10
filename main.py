# pygbag: webgl
import pygame
import sys
import asyncio
from gamestate import ChessBoard, WHITE, BLACK, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
from graphics.pieces import *
from graphics.button import Button
from graphics.animator import PieceAnimation
from graphics.sidebar import draw_sidebar
from graphics.rightside_screen import draw_rightside
from variables import *
from graphics.general_gfx import *

# Global Video Setup
screen = pygame.display.set_mode(
    (WINDOW_WIDTH, WINDOW_HEIGHT), 
    pygame.NOFRAME | pygame.HWSURFACE | pygame.DOUBLEBUF
)
pygame.display.set_caption("PyChess")

clock = pygame.time.Clock()

pygame.font.init()
log_font = pygame.font.SysFont("Calibri", 18, bold=True)
other_log_font = pygame.font.SysFont("Helvetica",100, bold=True)
font_title = pygame.font.SysFont("arial", 40, bold=True)

ogbackground = pygame.image.load('graphics/images/bg.jpg').convert()
background = pygame.transform.scale(ogbackground, (WINDOW_WIDTH, WINDOW_HEIGHT))
background.set_alpha(20)

board_size = TILE_SIZE * 8
board_mask = pygame.Surface((board_size, board_size), pygame.SRCALPHA).convert_alpha()
board_mask.fill((0, 0, 0, 0))
pygame.draw.rect(board_mask, (255, 255, 255, 255), (0, 0, board_size, board_size), border_radius=10)

STATIC_BOARD_SURFACE = pygame.Surface((board_size, board_size)).convert()
for row in range(8):
    for column in range(8):
        square_color = LIGHT_SQUARE_COLOR if (row + column) % 2 == 0 else DARK_SQUARE_COLOR
        pygame.draw.rect(STATIC_BOARD_SURFACE, square_color, (column * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE))

PROMOTION_OVERLAY = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA).convert_alpha()
PROMOTION_OVERLAY.fill((0, 0, 0, 150))

TEXT_CACHE = {}
def get_cached_text(text_string, font_object, color, bg_color=None):
    cache_key = f"{text_string}_{color}_{bg_color}"
    if cache_key not in TEXT_CACHE:
        TEXT_CACHE[cache_key] = font_object.render(text_string, True, color, bg_color)
    return TEXT_CACHE[cache_key]

def draw_chessboard(screen_surface, selected_square, last_move, game_board, in_animation, board_mask):
    board_size = TILE_SIZE * 8
    
    board_surface = STATIC_BOARD_SURFACE.copy()

    W_COLOR = 0  
    B_COLOR = 1  

    white_in_check = game_board.is_in_check(W_COLOR) if hasattr(game_board, 'is_in_check') else False
    black_in_check = game_board.is_in_check(B_COLOR) if hasattr(game_board, 'is_in_check') else False
    
    white_king_pos = game_board.find_king_position(W_COLOR) if hasattr(game_board, 'find_king_position') else None
    black_king_pos = game_board.find_king_position(B_COLOR) if hasattr(game_board, 'find_king_position') else None

    if last_move is not None:
        start_pos, end_pos = last_move
        pygame.draw.rect(board_surface, LAST_MOVE_HIGHLIGHT_COLOR, (start_pos[1]*TILE_SIZE, start_pos[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))
        pygame.draw.rect(board_surface, LAST_MOVE_HIGHLIGHT_COLOR, (end_pos[1]*TILE_SIZE, end_pos[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))

    if not in_animation and hasattr(game_board, 'king_in_check') and game_board.king_in_check:
        if white_in_check and white_king_pos:
            pygame.draw.rect(board_surface, CHECK_INDICATOR_COLOR, (white_king_pos[1]*TILE_SIZE, white_king_pos[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))
        if black_in_check and black_king_pos:
            pygame.draw.rect(board_surface, CHECK_INDICATOR_COLOR, (black_king_pos[1]*TILE_SIZE, black_king_pos[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))
    
    if selected_square is not None:
        pygame.draw.rect(board_surface, SELECTED_HIGHLIGHT_COLOR, (selected_square[1]*TILE_SIZE, selected_square[0]*TILE_SIZE, TILE_SIZE, TILE_SIZE))
                
    board_surface.blit(board_mask, (0, 0), special_flags=pygame.BLEND_RGBA_MIN)
    screen_surface.blit(board_surface, (BOARD_OFFSET_X, BOARD_OFFSET_Y))


def draw_captured_bars(screen_surface, captured_white, captured_black):
    PANEL_COLOR = (30, 30, 30)
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, WHITE_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT), 0, 0, 0, 0, 5, 5)
    pygame.draw.rect(screen_surface, PANEL_COLOR, (CAPTURE_BAR_X, BLACK_CAPTURE_BAR_Y, CAPTURE_BAR_WIDTH, CAPTURE_BAR_HEIGHT), 0, 0, 5, 5)

    def draw_mini_piece(piece, x, y): 
        radius = TILE_SIZE // 4
        fill_color = WHITE_PIECE_COLOR if piece.color == WHITE else BLACK_PIECE_COLOR
        outline_color = WHITE_PIECE_OUTLINE if piece.color == WHITE else BLACK_PIECE_OUTLINE
        draw_piece(piece.type, screen_surface, fill_color, outline_color, x, y, radius)

    for index, piece in enumerate(captured_white):
        column_offset = (index % 8) * (TILE_SIZE // 2)
        draw_mini_piece(piece, CAPTURE_BAR_X + 30 + column_offset, BLACK_CAPTURE_BAR_Y + CAPTURE_BAR_HEIGHT // 2 - CAPTURE_BAR_HEIGHT // 15)

    for index, piece in enumerate(captured_black):
        column_offset = (index % 8) * (TILE_SIZE // 2)
        draw_mini_piece(piece, CAPTURE_BAR_X + 30 + column_offset, WHITE_CAPTURE_BAR_Y + CAPTURE_BAR_HEIGHT // 2 - CAPTURE_BAR_HEIGHT // 15)


def draw_promotion_menu(screen_surface, turn_color):
    screen_surface.blit(PROMOTION_OVERLAY, (0, 0))

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

def draw_move_log_table(screen, move_history, mouse_pos, font):
    container_rect = pygame.Rect(PANEL_X - 10, PANEL_Y - 10, TILE_SIZE*5-20, TILE_SIZE*6)
    pygame.draw.rect(screen, (25,27,29), container_rect, border_radius=5)
    
    actual_moves = move_history[1:] 
    hovered_index = None
    total_pairs = (len(actual_moves) + 1) // 2

    for i in range(total_pairs):
        y_pos = PANEL_Y + (i * ROW_HEIGHT)
        
        num_text = get_cached_text(f"{i + 1}.", font, (118, 115, 111), (25,27,29))
        screen.blit(num_text, (PANEL_X, y_pos))
        
        white_idx = i * 2
        w_move_rect = pygame.Rect(PANEL_X + 30, y_pos, COL_WIDTH, ROW_HEIGHT - 4)
        w_bg_color = (38, 37, 34)

        if w_move_rect.collidepoint(mouse_pos):
            w_bg_color = (26, 25, 23) 
            hovered_index = white_idx + 1 

        pygame.draw.rect(screen, w_bg_color, w_move_rect, border_radius=3)
        w_text = get_cached_text(actual_moves[white_idx]["notation"].upper(), font, (248, 248, 248), w_bg_color)
        screen.blit(w_text, (PANEL_X + 35, y_pos + 2))

        black_idx = i * 2 + 1
        if black_idx < len(actual_moves):
            b_move_rect = pygame.Rect(PANEL_X + 80 + COL_WIDTH + 5, y_pos, COL_WIDTH, ROW_HEIGHT - 4)
            b_bg_color = (38, 37, 34)

            if b_move_rect.collidepoint(mouse_pos):
                b_bg_color = (26, 25, 23)
                hovered_index = black_idx + 1

            pygame.draw.rect(screen, b_bg_color, b_move_rect, border_radius=3)
            b_text = get_cached_text(actual_moves[black_idx]["notation"].upper(), font, (248, 248, 248), b_bg_color)
            screen.blit(b_text, (PANEL_X + 35 + COL_WIDTH + 5+50, y_pos + 2))
            
    return hovered_index

def draw_historical_pieces(screen, history_grid):
    for row in range(8):
        for col in range(8):
            data = history_grid[row][col]
            if data is not None:
                pixel_x = BOARD_OFFSET_X + (col * TILE_SIZE)
                pixel_y = BOARD_OFFSET_Y + (row * TILE_SIZE)
                
                if data["color"] == WHITE or data["color"] == "w":
                    fill, outline = (248, 248, 248), (45, 45, 45)
                else:
                    fill, outline = (86, 83, 82), (25, 25, 25)
                    
                radius = int(TILE_SIZE * 0.45)
                draw_piece(
                    data["type"], screen, fill, outline,
                    int(pixel_x + TILE_SIZE // 2), int(pixel_y + TILE_SIZE // 2),
                    radius, angle=0
                )

async def main():
    width, height = pygame.display.get_window_size()

    # Create the sidebar setup
    sidebar_bg = pygame.Surface((200, height), pygame.SRCALPHA).convert_alpha()
    sidebar_bg.fill((25, 27, 29, 160))

    sidebar_buttons = {
        "settings": Button(width-175, height-100, 150, 50, "Settings", (27, 29, 31), (35, 37, 39), font_size=20, border=False),
        "play": Button(width-175, 120, 150, 50, "Play", (27, 29, 31), (35, 37, 39), font_size=20, border=False),
        "game": Button(width-175, 180, 150, 50, "Game", (27, 29, 31), (35, 37, 39), font_size=20, border=False),
        "puzzles": Button(width-175, 240, 150, 50, "Puzzles", (27, 29, 31), (35, 37, 39), font_size=20, border=False)
    }

    game_board = ChessBoard()
    view_index = None
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
    
    action_buttons = {
        WHITE: {
            "resign": Button(RESIGN_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Resign", (27, 29, 31), (35, 37, 39), font_size=16),
            "draw": Button(DRAW_BTN_X, ACTION_BTN_Y_WHITE, ACTION_BTN_W, ACTION_BTN_H, "Draw", (27, 29, 31), (35, 37, 39), font_size=16)
        },
        BLACK: {
            "resign": Button(RESIGN_BTN_X, ACTION_BTN_Y_BLACK, ACTION_BTN_W, ACTION_BTN_H, "Resign", (27, 29, 31), (35, 37, 39), font_size=16),
            "draw": Button(DRAW_BTN_X, ACTION_BTN_Y_BLACK, ACTION_BTN_W, ACTION_BTN_H, "Draw", (27, 29, 31), (35, 37, 39), font_size=16)
        }
    }
        
    active_animation = None
    hidden_piece_data = None
    game_is_over = False

    hovered_history_index = None
    hover_cooldown_start = None
    hover_cooldown_duration = 500
    selected_piece_moves = []
    
    STATIC_INTERFACE_SURFACE = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT)).convert()
    STATIC_INTERFACE_SURFACE.blit(background, (0, 0))
    draw_rightside(STATIC_INTERFACE_SURFACE, other_log_font)
    draw_sidebar(STATIC_INTERFACE_SURFACE, sidebar_bg, sidebar_buttons)

    while is_game_running:
        if not DEBUGMODE:
            clock.tick(60) 
            
        mouse_pos = pygame.mouse.get_pos()
        current_turn = game_board.turn
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                is_game_running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    is_game_running = False
            
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if hovered_history_index is not None:
                    continue
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
                        game_is_over = False
                        continue
                    elif new_game_btn.is_clicked(mouse_pos, event.type):
                        game_board = ChessBoard()
                        selected_square = None
                        game_is_over = False
                        continue
                
                if not game_is_over:
                    if action_buttons[current_turn]["resign"].is_clicked(mouse_pos, event.type):
                        game_board.resign(game_board.turn)
                        game_is_over = True
                        selected_square = None
                        continue
                    elif action_buttons[current_turn]["draw"].is_clicked(mouse_pos, event.type):
                        game_board.declare_draw()
                        game_is_over = True
                        selected_square = None
                        continue

                clicked_column = (mouse_x - BOARD_OFFSET_X) // TILE_SIZE
                clicked_row = (mouse_y - BOARD_OFFSET_Y) // TILE_SIZE

                if 0 <= clicked_row < 8 and 0 <= clicked_column < 8:
                    if selected_square is None:
                        clicked_piece = game_board.grid[clicked_row][clicked_column]
                        if clicked_piece and clicked_piece.color == game_board.turn:
                            selected_square = (clicked_row, clicked_column)
                            selected_piece_moves = game_board.get_safe_legal_moves(clicked_row, clicked_column)
                    else:
                        start_row, start_column = selected_square
                        if (clicked_row, clicked_column) in selected_piece_moves:
                            moving_piece = game_board.grid[start_row][start_column]
                            hidden_piece_data = {
                                "row": clicked_row,
                                "col": clicked_column,
                                "piece": moving_piece
                            }
                            
                            active_animation = PieceAnimation(
                                moving_piece.type,
                                moving_piece.color,
                                (start_row, start_column),
                                (clicked_row, clicked_column),
                                TILE_SIZE,
                                board_offset,
                                speed=0.2
                            )
                            
                            is_pawn = moving_piece.type == 1
                            is_diagonal = start_column != clicked_column
                            is_target_empty = game_board.grid[clicked_row][clicked_column] is None
                            if is_pawn and is_diagonal and is_target_empty:
                                game_board.grid[start_row][clicked_column] = None

                            game_board.make_move(selected_square, (clicked_row, clicked_column))
                            game_board.grid[clicked_row][clicked_column] = None
                            
                        selected_square = None
                        selected_piece_moves = []
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
        
        raw_hover_index = draw_move_log_table(screen, game_board.move_history, mouse_pos, log_font)
        
        if raw_hover_index is not None:
            hovered_history_index = raw_hover_index
            hover_cooldown_start = None 
        else:
            if hovered_history_index is not None:
                ticks = pygame.time.get_ticks()
                if hover_cooldown_start is None:
                    hover_cooldown_start = ticks
            
                if ticks - hover_cooldown_start > hover_cooldown_duration:
                    hovered_history_index = None
                    hover_cooldown_start = None

        actual_moves_count = len(game_board.move_history) - 1

        if hovered_history_index is not None and hovered_history_index < actual_moves_count:
            target_idx = hovered_history_index + 1
            if target_idx >= actual_moves_count or hovered_history_index == (actual_moves_count - 1):
                target_idx = -1
            
            display_state = game_board.move_history[target_idx]
            viewing_last_move = display_state["last_move"]
            use_live = False
        else:
            viewing_last_move = game_board.last_move
            use_live = True
        
        screen.set_clip(pygame.Rect(BOARD_OFFSET_X, BOARD_OFFSET_Y, board_size, board_size))
        draw_chessboard(screen, selected_square, viewing_last_move, game_board, in_animation, board_mask)
        
        if not use_live:
            display_state = game_board.move_history[target_idx]
            draw_historical_pieces(screen, display_state["grid"])
        else:
            draw_pieces(screen, game_board.grid)
            if active_animation and active_animation.is_active:
                active_animation.update()
                
                fill, outline = ((248, 248, 248), (45, 45, 45)) if active_animation.color == WHITE else ((86, 83, 82), (25, 25, 25))
                radius = int(TILE_SIZE * 0.45)
                
                draw_piece(
                    active_animation.piece_type, screen, fill, outline,
                    int(active_animation.current_x), int(active_animation.current_y),
                    radius, angle=0 
                )
            elif hidden_piece_data:
                game_board.grid[hidden_piece_data["row"]][hidden_piece_data["col"]] = hidden_piece_data["piece"]
                hidden_piece_data = None
                active_animation = None
        
        if selected_square is not None and hovered_history_index is None:
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
            draw_promotion_menu(screen, game_board.turn)

        if game_is_over:
            draw_game_over_screen(screen, game_board.turn, [rematch_btn, new_game_btn], game_board, font_title)
        
        pygame.display.flip()
        await asyncio.sleep(0)
        
    pygame.quit()
    sys.exit()

def mainMenu():
    asyncio.run(main())
    
if __name__ == "__main__":
    mainMenu()