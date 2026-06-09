import pygame
from variables import *

def draw_rightside(screen,font):

    width, height = pygame.display.get_window_size()
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