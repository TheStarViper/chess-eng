import pygame

from gamestate import ChessBoard


class Button:
    def __init__(self, x, y, width, height, text, base_color, hover_color, text_color=(255, 255, 255), font_size=20):
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.base_color = base_color
        self.hover_color = hover_color
        self.text_color = text_color
        self.font = pygame.font.SysFont("arial", font_size, bold=True)
        self.is_hovered = False

    def is_hover(self, mouse_pos):
        if self.rect.collidepoint(mouse_pos):
            self.is_hovered = True
        else:
            self.is_hovered = False

    def draw(self, screen_surface):
        current_color = self.hover_color if self.is_hovered else self.base_color
        pygame.draw.rect(screen_surface, current_color, self.rect, border_radius=3)
        pygame.draw.rect(screen_surface, (255, 255, 255), self.rect, width=2, border_radius=3)
        
        text_surface = self.font.render(self.text, True, self.text_color)
        text_x = self.rect.x + (self.rect.width - text_surface.get_width()) // 2
        text_y = self.rect.y + (self.rect.height - text_surface.get_height()) // 2
        screen_surface.blit(text_surface, (text_x, text_y))

    def is_clicked(self, mouse_pos, event_type):
        return event_type == pygame.MOUSEBUTTONDOWN and self.rect.collidepoint(mouse_pos)
    
    def reposition(self, new_x, new_y):
        self.rect.x = new_x
        self.rect.y = new_y