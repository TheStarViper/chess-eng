import pygame
import math
import pygame.gfxdraw
#everything in this file is used to draw the pieces on the board and these graphics are made by AI

def draw_piece(piece, surface, fill_color, outline_color, center_x, center_y, radius):
    if piece == 1:
        pawn(surface, fill_color, outline_color, center_x, center_y, radius)
    elif piece == 2:
        knight(surface, fill_color, outline_color, center_x, center_y, radius)
    elif piece == 3:
        bishop(surface, fill_color, outline_color, center_x, center_y, radius)
    elif piece == 4:
        rook(surface, fill_color, outline_color, center_x, center_y, radius)
    elif piece == 5:
        queen(surface, fill_color, outline_color, center_x, center_y, radius)
    elif piece == 6:
        king(surface, fill_color, outline_color, center_x, center_y, radius)

def error(screen_surface, center_x, center_y, size=40): #ERROR MARKER
    top = (center_x, center_y - size // 2)
    bottom_left = (center_x - size // 2, center_y + size // 2)
    bottom_right = (center_x + size // 2, center_y + size // 2)
    
    points = [top, bottom_left, bottom_right]
    

    red_color = (220, 53, 69) 
    pygame.draw.polygon(screen_surface, red_color, points)
    

    outline_color = (50, 50, 50)
    pygame.draw.polygon(screen_surface, outline_color, points, 3)
    

    mark_color = (255, 255, 255)

    line_start = (center_x, center_y - size // 6)
    line_end = (center_x, center_y + size // 10)
    line_width = max(2, size // 12)
    pygame.draw.line(screen_surface, mark_color, line_start, line_end, line_width)
    
    dot_radius = max(2, size // 14)
    dot_y = center_y + size // 4
    pygame.draw.circle(screen_surface, mark_color, (center_x, dot_y), dot_radius)


def draw_aa_polygon(surface, points, fill_color, outline_color):
    """Draws a perfectly anti-aliased and filled polygon to eliminate jagged edges."""
    # Convert points to integers for gfxdraw
    int_points = [(int(x), int(y)) for x, y in points]
    pygame.gfxdraw.filled_polygon(surface, int_points, fill_color)
    pygame.gfxdraw.aapolygon(surface, int_points, outline_color)

def draw_modern_base(surface, fill_color, outline_color, center_x, center_y, radius):
    """Draws a sleek, slender, flat foundation block."""
    # Narrower width (1.20) to keep it from looking fat
    w = radius * 1.20
    h = radius * 0.20
    x = center_x - w / 2
    y = center_y + radius * 0.65
    
    # Base Rim
    pygame.draw.rect(surface, fill_color, (x, y, w, h), border_radius=2)
    pygame.draw.rect(surface, outline_color, (x, y, w, h), 2, border_radius=2)
    
    # High-contrast accent divider line
    pygame.draw.line(surface, outline_color, (center_x - w/2.2, y - 2), (center_x + w/2.2, y - 2), 3)

def pawn(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_modern_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Slender, tall elegant neck pillar
    body_points = [
        (center_x - radius * 0.15, center_y - radius * 0.25),
        (center_x + radius * 0.15, center_y - radius * 0.25),
        (center_x + radius * 0.38, center_y + radius * 0.65),
        (center_x - radius * 0.38, center_y + radius * 0.65)
    ]
    draw_aa_polygon(surface, body_points, fill_color, outline_color)
    
    # Perfectly anti-aliased head sphere sitting high up
    hx, hy, hr = int(center_x), int(center_y - radius * 0.45), int(radius * 0.28)
    pygame.gfxdraw.filled_circle(surface, hx, hy, hr, fill_color)
    pygame.gfxdraw.aacircle(surface, hx, hy, hr, outline_color)

def rook(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_modern_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Tall, strictly straight modern tower lines
    body_points = [
        (center_x - radius * 0.35, center_y - radius * 0.75), # Top Left
        (center_x + radius * 0.35, center_y - radius * 0.75), # Top Right
        (center_x + radius * 0.40, center_y + radius * 0.65), # Bottom Right
        (center_x - radius * 0.40, center_y + radius * 0.65)  # Bottom Left
    ]
    draw_aa_polygon(surface, body_points, fill_color, outline_color)
    
    # Minimalist clean vertical cutouts mapping the modern crenellations
    gap_w, gap_h = int(radius * 0.12), int(radius * 0.22)
    pygame.draw.rect(surface, outline_color, (int(center_x - radius * 0.22), int(center_y - radius * 0.76), gap_w, gap_h))
    pygame.draw.rect(surface, outline_color, (int(center_x + radius * 0.10), int(center_y - radius * 0.76), gap_w, gap_h))

def knight(surface, fill_color, outline_color, center_x, center_y, radius):
    # Base slab
    w = radius * 1.20
    pygame.draw.rect(surface, fill_color, (center_x - w/2, center_y + radius * 0.65, w, radius * 0.20), border_radius=2)
    pygame.draw.rect(surface, outline_color, (center_x - w/2, center_y + radius * 0.65, w, radius * 0.20), 2, border_radius=2)
    
    # Sharp, clean vector silhouette layout for the horse (Slender/Tall)
    points = [
        (center_x + radius * 0.35, center_y + radius * 0.65), 
        (center_x + radius * 0.35, center_y - radius * 0.2),  
        (center_x + radius * 0.12, center_y - radius * 0.85), # Max height
        (center_x - radius * 0.10, center_y - radius * 0.9),  # Pointy clean ear
        (center_x - radius * 0.45, center_y - radius * 0.5),  # Sleek nose top
        (center_x - radius * 0.48, center_y - radius * 0.2),  
        (center_x - radius * 0.18, center_y - radius * 0.05), # Sculpted jaw
        (center_x - radius * 0.02, center_y + radius * 0.18), # Inward chest curve
        (center_x - radius * 0.30, center_y + radius * 0.65), 
    ]
    draw_aa_polygon(surface, points, fill_color, outline_color)
    
    # Modern minimalist flat accent split down the mane
    shadow = [
        (center_x + radius * 0.35, center_y + radius * 0.65),
        (center_x + radius * 0.35, center_y - radius * 0.2),
        (center_x + radius * 0.12, center_y - radius * 0.85),
        (center_x + radius * 0.22, center_y - radius * 0.2),
        (center_x + radius * 0.22, center_y + radius * 0.65)
    ]
    draw_aa_polygon(surface, shadow, outline_color, outline_color)

def bishop(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_modern_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Slender architectural column base
    body_points = [
        (center_x - radius * 0.14, center_y - radius * 0.2),
        (center_x + radius * 0.14, center_y - radius * 0.2),
        (center_x + radius * 0.35, center_y + radius * 0.65),
        (center_x - radius * 0.35, center_y + radius * 0.65)
    ]
    draw_aa_polygon(surface, body_points, fill_color, outline_color)
    
    # Clean, elongated modern oval cap
    hx, hy, rx, ry = int(center_x), int(center_y - radius * 0.48), int(radius * 0.28), int(radius * 0.38)
    pygame.gfxdraw.filled_ellipse(surface, hx, hy, rx, ry, fill_color)
    pygame.gfxdraw.aaellipse(surface, hx, hy, rx, ry, outline_color)
    
    # Sharp negative-space diagonal geometric slit
    slice_points = [
        (center_x - radius * 0.05, center_y - radius * 0.6),
        (center_x + radius * 0.18, center_y - radius * 0.45),
        (center_x + radius * 0.10, center_y - radius * 0.42)
    ]
    draw_aa_polygon(surface, slice_points, outline_color, outline_color)

def queen(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_modern_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Sweeping, elegant slender torso lines
    body_points = [
        (center_x - radius * 0.15, center_y - radius * 0.2),
        (center_x + radius * 0.15, center_y - radius * 0.2),
        (center_x + radius * 0.38, center_y + radius * 0.65),
        (center_x - radius * 0.38, center_y + radius * 0.65)
    ]
    draw_aa_polygon(surface, body_points, fill_color, outline_color)
    
    # Sharp, clean structural vector crown stretching high up
    crown_points = [
        (center_x - radius * 0.35, center_y - radius * 0.75), # Left peak
        (center_x - radius * 0.15, center_y - radius * 0.35), # Valley
        (center_x,                 center_y - radius * 0.88), # High central apex
        (center_x + radius * 0.15, center_y - radius * 0.35), # Valley
        (center_x + radius * 0.35, center_y - radius * 0.75), # Right peak
        (center_x + radius * 0.22, center_y - radius * 0.2),
        (center_x - radius * 0.22, center_y - radius * 0.2)
    ]
    draw_aa_polygon(surface, crown_points, fill_color, outline_color)

def king(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_modern_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Solid, stately regal core
    body_points = [
        (center_x - radius * 0.18, center_y - radius * 0.2),
        (center_x + radius * 0.18, center_y - radius * 0.2),
        (center_x + radius * 0.38, center_y + radius * 0.65),
        (center_x - radius * 0.38, center_y + radius * 0.65)
    ]
    draw_aa_polygon(surface, body_points, fill_color, outline_color)
    
    # Flat-top block crown cap with rounded styling
    cx, cy, cw, ch = int(center_x - radius * 0.32), int(center_y - radius * 0.68), int(radius * 0.64), int(radius * 0.48)
    pygame.draw.rect(surface, fill_color, (cx, cy, cw, ch), border_radius=4)
    pygame.draw.rect(surface, outline_color, (cx, cy, cw, ch), 2, border_radius=4)
    
    # Sharp, minimalist geometric cross finial piercing the top boundary line
    pygame.draw.rect(surface, fill_color, (int(center_x - radius * 0.06), int(center_y - radius * 0.92), int(radius * 0.12), int(radius * 0.26))) 
    pygame.draw.rect(surface, fill_color, (int(center_x - radius * 0.18), int(center_y - radius * 0.82), int(radius * 0.36), int(radius * 0.08))) 
    pygame.draw.rect(surface, outline_color, (int(center_x - radius * 0.06), int(center_y - radius * 0.92), int(radius * 0.12), int(radius * 0.26)), 1)
    pygame.draw.rect(surface, outline_color, (int(center_x - radius * 0.18), int(center_y - radius * 0.82), int(radius * 0.36), int(radius * 0.08)), 1)