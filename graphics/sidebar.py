import pygame
from graphics.button import Button

def draw_sidebar(screen_surface):
    width, height = pygame.display.get_window_size()
    mouse_pos = pygame.mouse.get_pos()

    #pygame.draw.rect(screen_surface, (25, 27, 29) , (width-200,0,200,height)) #sidebar background
    rect_surface = pygame.Surface((200, height), pygame.SRCALPHA)
    rect_surface.fill((25, 27, 29, 160))
    screen_surface.blit(rect_surface, (width-200, 0))

    settings = Button(width-175, height-100, 150, 50, "Settings", (27, 29, 31), (35, 37, 39), font_size= 20, border = False)
    settings.is_hover(mouse_pos)
    settings.draw(screen_surface)

    play = Button(width-175, 120, 150, 50, "Play", (27, 29, 31), (35, 37, 39), font_size= 20, border = False)
    play.is_hover(mouse_pos)
    play.draw(screen_surface)

    game = Button(width-175, 180, 150, 50, "Game", (27, 29, 31), (35, 37, 39), font_size= 20, border = False)
    game.is_hover(mouse_pos)
    game.draw(screen_surface)

    puzzles = Button(width-175, 240, 150, 50, "Puzzles", (27, 29, 31), (35, 37, 39), font_size= 20, border = False)
    puzzles.is_hover(mouse_pos)
    puzzles.draw(screen_surface)