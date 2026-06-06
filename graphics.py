import pygame

def pawn(screen_surface, fill_color, outline_color, center_x, center_y, radius):
    pygame.draw.circle(screen_surface, fill_color, (center_x, center_y + 10), radius - 5)
    pygame.draw.circle(screen_surface, outline_color, (center_x, center_y + 10), radius - 5, 3)
    pygame.draw.circle(screen_surface, fill_color, (center_x, center_y - 10), radius - 12)
    pygame.draw.circle(screen_surface, outline_color, (center_x, center_y - 10), radius - 12, 3)

def knight(screen_surface, fill_color, outline_color, center_x, center_y, radius):
    pass

def bishop(screen_surface, fill_color, outline_color, center_x, center_y, radius):
    pass

def rook(screen_surface, fill_color, outline_color, center_x, center_y, radius):
    pass

def queen(screen_surface, fill_color, outline_color, center_x, center_y, radius):
    pass

def king(screen_surface, fill_color, outline_color, center_x, center_y, radius):
    pass