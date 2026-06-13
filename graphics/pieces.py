import pygame
from variables import pieces
PIECE_CACHE = {}
#https://www.dafont.com/chess.font

def pre_render_pieces(font_object, white_fill, black_fill, outline_color, thickness=2):
    global PIECE_CACHE
    symbols = {1: "o", 2: "j", 3: "n", 4: "t", 5: "w", 6: "l"}
    
    for piece_id, symbol in symbols.items():
        temp_core = font_object.render(symbol, True, (255, 255, 255))
        
        padding = thickness + 1
        canvas_width = temp_core.get_width() + padding * 2
        canvas_height = temp_core.get_height() + padding * 2
        
        mask = pygame.mask.from_surface(temp_core)
        mask_surf = mask.to_surface(setcolor=outline_color, unsetcolor=(0, 0, 0, 0))
        
        local_x, local_y = padding, padding
        offsets = [
            (-thickness, 0), (thickness, 0), (0, -thickness), (0, thickness),
            (-thickness, -thickness), (-thickness, thickness), 
            (thickness, -thickness), (thickness, thickness)
        ]

        white_canvas = pygame.Surface((canvas_width, canvas_height), pygame.SRCALPHA)
        for dx, dy in offsets:
            white_canvas.blit(mask_surf, (local_x + dx, local_y + dy))
        white_core = font_object.render(symbol, True, white_fill)
        white_canvas.blit(white_core, (local_x, local_y))
        PIECE_CACHE[(piece_id, "WHITE")] = white_canvas

        black_canvas = pygame.Surface((canvas_width, canvas_height), pygame.SRCALPHA)
        for dx, dy in offsets:
            black_canvas.blit(mask_surf, (local_x + dx, local_y + dy))
        black_core = font_object.render(symbol, True, black_fill)
        black_canvas.blit(black_core, (local_x, local_y))
        PIECE_CACHE[(piece_id, "BLACK")] = black_canvas

def draw_piece(piece, surface, fill_color, center_x, center_y, side, angle=0):
    side_str = "WHITE" if str(side).upper().strip() in ("WHITE", "W", "1", "TRUE") else "BLACK"
    
    piece_image = PIECE_CACHE.get((piece, side_str))
    
    if piece_image is None:
        return

    if angle != 0:
        piece_image = pygame.transform.rotate(piece_image, angle)

    rect = piece_image.get_rect(center=(center_x, center_y))
    surface.blit(piece_image, rect)