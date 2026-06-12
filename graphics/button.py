import pygame
import os
from gamestate import ChessBoard


class Button:
    def __init__(self, 
                x, 
                y, 
                width, 
                height, 
                text, 
                base_color, 
                hover_color, 
                text_color=(255, 255, 255), 
                font_size=20, 
                border = True, 
                border_color = (255,255,255),
                border_width = 3,
                action_type = None,
                image_path = None):  # <-- Added image_path parameter
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.base_color = base_color
        self.hover_color = hover_color
        self.text_color = text_color
        self.font = pygame.font.SysFont("arial", font_size, bold=True)
        self.is_hovered = False
        self.border = border
        self.border_color = border_color
        self.border_width = border_width
        self.action_type = action_type
        
        self.sprite = None
        if image_path and os.path.exists(image_path):
            try:
                raw_image = pygame.image.load(image_path).convert_alpha()
                icon_padding = 8
                icon_size = height - icon_padding
                self.sprite = pygame.transform.smoothscale(raw_image, (icon_size, icon_size))
            except pygame.error as e:
                print(f"Error loading button sprite asset {image_path}: {e}")

    def is_hover(self, mouse_pos):
        if self.rect.collidepoint(mouse_pos):
            self.is_hovered = True
        else:
            self.is_hovered = False

    def draw(self, screen):
        mouse_pos = pygame.mouse.get_pos()
        is_hovered = self.rect.collidepoint(mouse_pos)
        
        bg_color = self.hover_color if is_hovered else self.base_color
        
        if is_hovered:
            if self.action_type == "resign":
                bg_color = (70, 35, 35)     
            elif self.action_type == "draw":
                bg_color = (35, 55, 70)     

        pygame.draw.rect(screen, bg_color, self.rect, border_radius=4)

        if self.border:
            b_color = self.border_color
            if is_hovered:
                if self.action_type == "resign": b_color = (255, 120, 120)
                elif self.action_type == "draw": b_color = (120, 200, 255)
            pygame.draw.rect(screen, b_color, self.rect, width=self.border_width, border_radius=4)

        if self.sprite:
            if self.action_type == "resign":
                icon_color = (255, 120, 120) if is_hovered else (180, 185, 190)
            elif self.action_type == "draw":
                icon_color = (120, 200, 255) if is_hovered else (180, 185, 190)
            else:
                icon_color = self.text_color
                
            self.draw_recolored_sprite(screen, self.sprite, self.rect, icon_color)
        else:
            text_surf = self.font.render(self.text, True, self.text_color)
            screen.blit(text_surf, text_surf.get_rect(center=self.rect.center))

    def is_clicked(self, mouse_pos, event_type):
        return event_type == pygame.MOUSEBUTTONDOWN and self.rect.collidepoint(mouse_pos)
    
    def reposition(self, new_x, new_y):
        self.rect.x = new_x
        self.rect.y = new_y

    def draw_recolored_sprite(self, screen, surface, target_rect, icon_color):
        if not surface:
            return
        
        mask_surf = pygame.Surface(surface.get_size(), pygame.SRCALPHA)
        
        mask_surf.fill(icon_color)
        
        mask = pygame.mask.from_surface(surface)
        mask_surface = mask.to_surface(setcolor=icon_color, unsetcolor=(0, 0, 0, 0))
        mask_surface.set_colorkey((0, 0, 0))
        screen.blit(mask_surface, mask_surface.get_rect(center=target_rect.center))