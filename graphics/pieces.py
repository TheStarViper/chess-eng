import pygame
import pygame.gfxdraw
import math

def rotate_point(px, py, cx, cy, angle_degrees):
    """Rotates a point (px, py) around a center (cx, cy) by an angle in degrees."""
    if angle_degrees == 0:
        return (px, py)
    
    # Convert angle to radians
    angle_rad = math.radians(angle_degrees)
    cos_a = math.cos(angle_rad)
    sin_a = math.sin(angle_rad)
    
    # Translate point back to origin
    dx = px - cx
    dy = py - cy
    
    # Apply standard 2D rotation matrix math
    rx = dx * cos_a - dy * sin_a
    ry = dx * sin_a + dy * cos_a
    
    # Translate back to the absolute square center coordinates
    return (int(rx + cx), int(ry + cy))


def draw_piece(piece, surface, fill_color, outline_color, center_x, center_y, radius, angle=0):
    """
    Main router to draw chess pieces with a modern vector aesthetic.
    Supports an 'angle' argument in degrees for dynamic tilting effects.
    """
    if piece == 1:
        pawn(surface, fill_color, outline_color, center_x, center_y, radius, angle)
    elif piece == 2:
        knight(surface, fill_color, outline_color, center_x, center_y, radius, angle)
    elif piece == 3:
        bishop(surface, fill_color, outline_color, center_x, center_y, radius, angle)
    elif piece == 4:
        rook(surface, fill_color, outline_color, center_x, center_y, radius, angle)
    elif piece == 5:
        queen(surface, fill_color, outline_color, center_x, center_y, radius, angle)
    elif piece == 6:
        king(surface, fill_color, outline_color, center_x, center_y, radius, angle)


# --- ANTI-ALIASED ROTATION HELPERS ---
def draw_aa_polygon(surface, points, cx, cy, angle, fill_color, outline_color):
    """Rotates all polygon matrix coordinates before rendering."""
    rotated_points = [rotate_point(x, y, cx, cy, angle) for x, y in points]
    pygame.gfxdraw.filled_polygon(surface, rotated_points, fill_color)
    pygame.gfxdraw.aapolygon(surface, rotated_points, outline_color)

def draw_aa_circle(surface, cx, cy, r, pivot_x, pivot_y, angle, fill_color, outline_color):
    """Rotates a circular center point around the primary pivot anchor."""
    rx, ry = rotate_point(cx, cy, pivot_x, pivot_y, angle)
    pygame.gfxdraw.filled_circle(surface, rx, ry, r, fill_color)
    pygame.gfxdraw.aacircle(surface, rx, ry, r, outline_color)


# --- PIECE DESIGNS COUPLING ROTATION PATTERNS ---

def pawn(surface, fill, out, cx, cy, r, angle):
    draw_aa_polygon(surface, [(cx - r*0.6, cy + r*0.8), (cx + r*0.6, cy + r*0.8), 
                             (cx + r*0.5, cy + r*0.6), (cx - r*0.5, cy + r*0.6)], cx, cy, angle, fill, out)
    draw_aa_polygon(surface, [(cx - r*0.35, cy + r*0.6), (cx + r*0.35, cy + r*0.6), 
                             (cx + r*0.15, cy - r*0.1), (cx - r*0.15, cy - r*0.1)], cx, cy, angle, fill, out)
    draw_aa_circle(surface, cx, int(cy - r*0.25), int(r * 0.35), cx, cy, angle, fill, out)


def rook(surface, fill, out, cx, cy, r, angle):
    points = [
        (cx - r*0.6, cy + r*0.8), (cx + r*0.6, cy + r*0.8),
        (cx + r*0.5, cy + r*0.6), (cx + r*0.4, cy - r*0.3),
        (cx + r*0.5, cy - r*0.3), (cx + r*0.5, cy - r*0.7),
        (cx + r*0.25, cy - r*0.7), (cx + r*0.25, cy - r*0.5),
        (cx + r*0.1, cy - r*0.5), (cx + r*0.1, cy - r*0.7),
        (cx - r*0.1, cy - r*0.7), (cx - r*0.1, cy - r*0.5),
        (cx - r*0.25, cy - r*0.5), (cx - r*0.25, cy - r*0.7),
        (cx - r*0.5, cy - r*0.7), (cx - r*0.5, cy - r*0.3),
        (cx - r*0.4, cy - r*0.3), (cx - r*0.5, cy + r*0.6)
    ]
    draw_aa_polygon(surface, points, cx, cy, angle, fill, out)


def knight(surface, fill, out, cx, cy, r, angle):
    points = [
        (cx - r*0.5, cy + r*0.8), (cx + r*0.5, cy + r*0.8), 
        (cx + r*0.4, cy + r*0.5), (cx + r*0.3, cy - r*0.1), 
        (cx + r*0.4, cy - r*0.5), (cx + r*0.2, cy - r*0.7), 
        (cx + r*0.1, cy - r*0.7), (cx + r*0.15, cy - r*0.4), 
        (cx - r*0.1, cy - r*0.5), (cx - r*0.4, cy - r*0.3), 
        (cx - r*0.5, cy - r*0.1), (cx - r*0.2, cy + r*0.0), 
        (cx - r*0.4, cy + r*0.2), (cx - r*0.1, cy + r*0.1), 
        (cx - r*0.4, cy + r*0.6)
    ]
    draw_aa_polygon(surface, points, cx, cy, angle, fill, out)
    draw_aa_circle(surface, int(cx - r*0.15), int(cy - r*0.35), max(2, int(r*0.06)), cx, cy, angle, out, out)


def bishop(surface, fill, out, cx, cy, r, angle):
    draw_aa_polygon(surface, [(cx - r*0.55, cy + r*0.8), (cx + r*0.55, cy + r*0.8), 
                             (cx + r*0.45, cy + r*0.6), (cx - r*0.45, cy + r*0.6)], cx, cy, angle, fill, out)
    points = []
    for deg in range(0, 361, 20):
        rad = math.radians(deg)
        x = cx + int(r * 0.4 * math.cos(rad))
        y = cy + int(r * 0.5 * math.sin(rad)) - int(r*0.05)
        points.append((x, y))
    draw_aa_polygon(surface, points, cx, cy, angle, fill, out)
    draw_aa_circle(surface, cx, int(cy - r*0.6), int(r * 0.1), cx, cy, angle, fill, out)
    
    # Slit cut calculation line
    p1 = rotate_point(cx, int(cy - r*0.4), cx, cy, angle)
    p2 = rotate_point(int(cx + r*0.25), int(cy - r*0.15), cx, cy, angle)
    pygame.draw.line(surface, out, p1, p2, max(2, int(r*0.06)))


def queen(surface, fill, out, cx, cy, r, angle):
    draw_aa_polygon(surface, [(cx - r*0.6, cy + r*0.8), (cx + r*0.6, cy + r*0.8), 
                             (cx + r*0.5, cy + r*0.6), (cx - r*0.5, cy + r*0.6)], cx, cy, angle, fill, out)
    points = [
        (cx - r*0.4, cy + r*0.6), (cx + r*0.4, cy + r*0.6),
        (cx + r*0.5, cy - r*0.3), (cx + r*0.25, cy + r*0.1),
        (cx, cy - r*0.5), 
        (cx - r*0.25, cy + r*0.1), (cx - r*0.5, cy - r*0.3)
    ]
    draw_aa_polygon(surface, points, cx, cy, angle, fill, out)
    draw_aa_circle(surface, int(cx - r*0.5), int(cy - r*0.3), int(r*0.08), cx, cy, angle, fill, out)
    draw_aa_circle(surface, cx, int(cy - r*0.5), int(r*0.08), cx, cy, angle, fill, out)
    draw_aa_circle(surface, int(cx + r*0.5), int(cy - r*0.3), int(r*0.08), cx, cy, angle, fill, out)


def king(surface, fill, out, cx, cy, r, angle):
    draw_aa_polygon(surface, [(cx - r*0.6, cy + r*0.8), (cx + r*0.6, cy + r*0.8), 
                              (cx + r*0.5, cy + r*0.6), (cx - r*0.5, cy + r*0.6)], cx, cy, angle, fill, out)
    points = [
        (cx - r*0.4, cy + r*0.6), (cx + r*0.4, cy + r*0.6),
        (cx + r*0.45, cy - r*0.4), (cx + r*0.15, cy - r*0.3),
        (cx, cy - r*0.45),
        (cx - r*0.15, cy - r*0.3), (cx - r*0.45, cy - r*0.4)
    ]
    draw_aa_polygon(surface, points, cx, cy, angle, fill, out)
    
    thick = max(2, int(r * 0.08))
    # Pivot vector tracking lines for top cross structure
    l1_s = rotate_point(cx, int(cy - r*0.45), cx, cy, angle)
    l1_e = rotate_point(cx, int(cy - r*0.75), cx, cy, angle)
    l2_s = rotate_point(int(cx - r*0.15), int(cy - r*0.6), cx, cy, angle)
    l2_e = rotate_point(int(cx + r*0.15), int(cy - r*0.6), cx, cy, angle)
    
    pygame.draw.line(surface, out, l1_s, l1_e, thick + 2)
    pygame.draw.line(surface, out, l2_s, l2_e, thick + 2)
    pygame.draw.line(surface, fill, l1_s, l1_e, thick)
    pygame.draw.line(surface, fill, l2_s, l2_e, thick)