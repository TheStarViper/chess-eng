import pygame
import math

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


def draw_piece_base(surface, fill_color, outline_color, center_x, center_y, radius):
    """Draws a consistent bottom pedestal for all major pieces."""
    # Base rectangle width and height proportional to the piece radius
    base_w = radius * 1.5
    base_h = radius * 0.4
    base_x = center_x - base_w // 2
    base_y = center_y + radius * 0.6
    
    pygame.draw.rect(surface, fill_color, (base_x, base_y, base_w, base_h), border_radius=3)
    pygame.draw.rect(surface, outline_color, (base_x, base_y, base_w, base_h), 3, border_radius=3)


def pawn(surface, fill_color, outline_color, center_x, center_y, radius):
    # Standard droplet/pawn shape: A tear-shaped body with a round head
    body_points = [
        (center_x, center_y - radius * 0.2),
        (center_x - radius * 0.6, center_y + radius * 0.8),
        (center_x + radius * 0.6, center_y + radius * 0.8)
    ]
    pygame.draw.polygon(surface, fill_color, body_points)
    pygame.draw.polygon(surface, outline_color, body_points, 3)
    
    # Head
    pygame.draw.circle(surface, fill_color, (center_x, center_y - radius * 0.3), radius * 0.4)
    pygame.draw.circle(surface, outline_color, (center_x, center_y - radius * 0.3), radius * 0.4, 3)


def rook(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_piece_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Castle tower body
    body_w = radius * 1.1
    body_h = radius * 1.1
    body_x = center_x - body_w // 2
    body_y = center_y - radius * 0.5
    
    pygame.draw.rect(surface, fill_color, (body_x, body_y, body_w, body_h))
    pygame.draw.rect(surface, outline_color, (body_x, body_y, body_w, body_h), 3)
    
    # Crenellations (the battlements on top)
    # Left, middle, and right cutouts
    top_y = body_y - 4
    pygame.draw.line(surface, outline_color, (body_x + 3, body_y), (body_x + 3, top_y), 4)
    pygame.draw.line(surface, outline_color, (center_x, body_y), (center_x, top_y), 4)
    pygame.draw.line(surface, outline_color, (body_x + body_w - 3, body_y), (body_x + body_w - 3, top_y), 4)


def knight(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_piece_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # A stylized geometric horse profile pointing left
    points = [
        (center_x + radius * 0.5, center_y + radius * 0.6),  # Back bottom
        (center_x + radius * 0.5, center_y - radius * 0.2),  # Mane top
        (center_x, center_y - radius * 0.7),                 # Ears
        (center_x - radius * 0.6, center_y - radius * 0.4),  # Snout top
        (center_x - radius * 0.6, center_y - radius * 0.1),  # Snout bottom
        (center_x - radius * 0.1, center_y + radius * 0.1),  # Jaw
        (center_x - radius * 0.4, center_y + radius * 0.6),  # Chest bottom
    ]
    pygame.draw.polygon(surface, fill_color, points)
    pygame.draw.polygon(surface, outline_color, points, 3)


def bishop(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_piece_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Oval Mitre body
    body_rect = (center_x - radius * 0.55, center_y - radius * 0.6, radius * 1.1, radius * 1.2)
    pygame.draw.ellipse(surface, fill_color, body_rect)
    pygame.draw.ellipse(surface, outline_color, body_rect, 3)
    
    # Small cross on top
    cross_y = center_y - radius * 0.75
    pygame.draw.line(surface, outline_color, (center_x, cross_y - 6), (center_x, cross_y + 4), 2)
    pygame.draw.line(surface, outline_color, (center_x - 5, cross_y - 2), (center_x + 5, cross_y - 2), 2)
    
    # The traditional Bishop's slice/slit
    pygame.draw.line(surface, outline_color, (center_x - radius * 0.2, center_y - radius * 0.2), (center_x + radius * 0.3, center_y - radius * 0.5), 2)


def queen(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_piece_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Spiked crown using a polygon
    crown_points = [
        (center_x - radius * 0.7, center_y + radius * 0.6), # bottom left
        (center_x - radius * 0.8, center_y - radius * 0.4), # left spike
        (center_x - radius * 0.3, center_y),                # valley
        (center_x, center_y - radius * 0.6),                # center tall spike
        (center_x + radius * 0.3, center_y),                # valley
        (center_x + radius * 0.8, center_y - radius * 0.4), # right spike
        (center_x + radius * 0.7, center_y + radius * 0.6), # bottom right
    ]
    pygame.draw.polygon(surface, fill_color, crown_points)
    pygame.draw.polygon(surface, outline_color, crown_points, 3)
    
    # Tiny jewels on top of the spikes
    for cx, cy in [(center_x - radius * 0.8, center_y - radius * 0.4), 
                   (center_x, center_y - radius * 0.6), 
                   (center_x + radius * 0.8, center_y - radius * 0.4)]:
        pygame.draw.circle(surface, outline_color, (cx, cy), 3)


def king(surface, fill_color, outline_color, center_x, center_y, radius):
    draw_piece_base(surface, fill_color, outline_color, center_x, center_y, radius)
    
    # Majestic robed / blocky crown body
    points = [
        (center_x - radius * 0.6, center_y + radius * 0.6),
        (center_x - radius * 0.7, center_y - radius * 0.4),
        (center_x - radius * 0.3, center_y - radius * 0.3),
        (center_x + radius * 0.3, center_y - radius * 0.3),
        (center_x + radius * 0.7, center_y - radius * 0.4),
        (center_x + radius * 0.6, center_y + radius * 0.6),
    ]
    pygame.draw.polygon(surface, fill_color, points)
    pygame.draw.polygon(surface, outline_color, points, 3)
    
    # Large Royal Cross on top
    cross_y = center_y - radius * 0.65
    pygame.draw.line(surface, outline_color, (center_x, cross_y - 10), (center_x, cross_y + 4), 4)
    pygame.draw.line(surface, outline_color, (center_x - 7, cross_y - 3), (center_x + 7, cross_y - 3), 4)