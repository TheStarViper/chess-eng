import pygame
from graphics.button import Button

def draw_sidebar(screen_surface, sidebar_bg, sidebar_buttons):
    width, height = pygame.display.get_window_size()
    mouse_pos = pygame.mouse.get_pos()

    # Blit the pre-calculated background surface instantly
    screen_surface.blit(sidebar_bg, (width-200, 0))

    # Update hover states and draw the pre-existing buttons
    for button in sidebar_buttons.values():
        button.is_hover(mouse_pos)
        button.draw(screen_surface)