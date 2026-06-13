import pygame
from graphics.button import Button

def draw_sidebar(screen_surface, sidebar_bg, sidebar_buttons):
    width, height = pygame.display.get_window_size()
    mouse_pos = pygame.mouse.get_pos()

    screen_surface.blit(sidebar_bg, (width-200, 0))

    for button in sidebar_buttons.values():
        button.is_hover(mouse_pos)
        button.draw(screen_surface)